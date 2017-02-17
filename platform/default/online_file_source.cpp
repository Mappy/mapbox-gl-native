#include <mbgl/storage/online_file_source.hpp>
#include <mbgl/storage/http_file_source.hpp>
#include <mbgl/storage/network_status.hpp>

#include <mbgl/storage/response.hpp>
#include <mbgl/platform/log.hpp>

#include <mbgl/util/constants.hpp>
#include <mbgl/util/thread.hpp>
#include <mbgl/util/mapbox.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/async_task.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/timer.hpp>
#include <mbgl/util/http_timeout.hpp>
#include <mbgl/platform/log.hpp>
#include <mbgl/platform/log.hpp>

#include <algorithm>
#include <cassert>
#include <list>
#include <unordered_set>
#include <unordered_map>

namespace mbgl {

class OnlineFileRequest : public AsyncRequest {
public:
    using Callback = std::function<void (Response)>;

    OnlineFileRequest(Resource, Callback, OnlineFileSource::Impl&);
    ~OnlineFileRequest() override;

    void networkIsReachableAgain();
    void schedule(optional<Timestamp> expires);
    void completed(Response);

    OnlineFileSource::Impl& impl;
    Resource resource;
    std::unique_ptr<AsyncRequest> request;
    util::Timer timer;
    Callback callback;

    // Counts the number of times a response was already expired when received. We're using
    // this to add a delay when making a new request so we don't keep retrying immediately
    // in case of a server serving expired tiles.
    uint32_t expiredRequests = 0;

    // Counts the number of subsequent failed requests. We're using this value for exponential
    // backoff when retrying requests.
    uint32_t failedRequests = 0;
    Response::Error::Reason failedRequestReason = Response::Error::Reason::Success;
    optional<Timestamp> retryAfter;
};

class OnlineFileSource::Impl {
public:
    Impl() {
        NetworkStatus::Subscribe(&reachability);
    }

    ~Impl() {
        NetworkStatus::Unsubscribe(&reachability);
    }

    void add(OnlineFileRequest* request) {
        Log::Error(Event::Setup, "OnlineFileSource add OnlineFileRequest %p %s", request, request->resource.url.c_str());
        allRequests.insert(request);
    }

    void remove(OnlineFileRequest* request) {
        Log::Error(Event::Setup, "OnlineFileSource remove OnlineFileRequest %p %s", request, request->resource.url.c_str());
        allRequests.erase(request);
        if (activeRequests.erase(request)) {
            Log::Error(Event::Setup, "OnlineFileSource remove erased from activeRequests %i", activeRequests.size());
            activatePendingRequest();
        } else {
            auto it = pendingRequestsMap.find(request);
            if (it != pendingRequestsMap.end()) {
                pendingRequestsList.erase(it->second);
                pendingRequestsMap.erase(it);
                Log::Error(Event::Setup, "OnlineFileSource remove erased from pendingRequestsMap %i", pendingRequestsList.size());
            }
        }
        assert(pendingRequestsMap.size() == pendingRequestsList.size());
    }

    void activateOrQueueRequest(OnlineFileRequest* request) {
        assert(allRequests.find(request) != allRequests.end());
        assert(activeRequests.find(request) == activeRequests.end());
        assert(!request->request);

        Log::Error(Event::Setup, "activateOrQueueRequest::activeRequests.size() %i", activeRequests.size());
        if (activeRequests.size() >= HTTPFileSource::maximumConcurrentRequests()) {
            queueRequest(request);
        } else {
            activateRequest(request);
        }
    }

    void queueRequest(OnlineFileRequest* request) {
        
        Log::Error(Event::Setup, "Ok queue %p", request);

        auto it = pendingRequestsList.insert(pendingRequestsList.end(), request);
        pendingRequestsMap.emplace(request, std::move(it));
        assert(pendingRequestsMap.size() == pendingRequestsList.size());
    }

    void activateRequest(OnlineFileRequest* request) {
       
        activeRequests.insert(request);
        request->request = httpFileSource.request(request->resource, [=] (Response response) {
             Log::Error(Event::Setup, "start callback %p", request);
            activeRequests.erase(request);
            activatePendingRequest();
            request->request.reset();
             Log::Error(Event::Setup, "reponse completed");
            request->completed(response);
        });
         Log::Error(Event::Setup, "Ok activateRequest %p", request);
        assert(pendingRequestsMap.size() == pendingRequestsList.size());
    }

    void activatePendingRequest() {
        if (pendingRequestsList.empty()) {
            return;
        }
        
       

        OnlineFileRequest* request = pendingRequestsList.front();
        pendingRequestsList.pop_front();

        pendingRequestsMap.erase(request);
        
        Log::Error(Event::Setup, "pendingRequestsList count %i", pendingRequestsMap.size());
        
       Log::Error(Event::Setup, "pop pending %p", request);

        activateRequest(request);
        assert(pendingRequestsMap.size() == pendingRequestsList.size());
    }
    
    bool isPending(OnlineFileRequest* request) {
        return pendingRequestsMap.find(request) != pendingRequestsMap.end();
    }
    
    bool isActive(OnlineFileRequest* request) {
        return activeRequests.find(request) != activeRequests.end();
    }

private:
    void networkIsReachableAgain() {
        for (auto& request : allRequests) {
            request->networkIsReachableAgain();
        }
    }

    /**
     * The lifetime of a request is:
     *
     * 1. Waiting for timeout (revalidation or retry)
     * 2. Pending (waiting for room in the active set)
     * 3. Active (open network connection)
     * 4. Back to #1
     *
     * Requests in any state are in `allRequests`. Requests in the pending state are in
     * `pendingRequests`. Requests in the active state are in `activeRequests`.
     */
    std::unordered_set<OnlineFileRequest*> allRequests;
    std::list<OnlineFileRequest*> pendingRequestsList;
    std::unordered_map<OnlineFileRequest*, std::list<OnlineFileRequest*>::iterator> pendingRequestsMap;
    std::unordered_set<OnlineFileRequest*> activeRequests;

    HTTPFileSource httpFileSource;
    util::AsyncTask reachability { std::bind(&Impl::networkIsReachableAgain, this) };
};

OnlineFileSource::OnlineFileSource()
    : impl(std::make_unique<Impl>()) {
}

OnlineFileSource::~OnlineFileSource() = default;

std::unique_ptr<AsyncRequest> OnlineFileSource::request(const Resource& resource, Callback callback) {
    Resource res = resource;
    
    //Log::Error(Event::Setup, "Resource %i",resource.kind);

    switch (resource.kind) {
    case Resource::Kind::Unknown:
        break;

    case Resource::Kind::Style:
        res.url = mbgl::util::mapbox::normalizeStyleURL(apiBaseURL, resource.url, accessToken);
        break;

    case Resource::Kind::Source:
        res.url = util::mapbox::normalizeSourceURL(apiBaseURL, resource.url, accessToken);
        break;

    case Resource::Kind::Glyphs:
        res.url = util::mapbox::normalizeGlyphsURL(apiBaseURL, resource.url, accessToken);
        break;

    case Resource::Kind::SpriteImage:
    case Resource::Kind::SpriteJSON:
        res.url = util::mapbox::normalizeSpriteURL(apiBaseURL, resource.url, accessToken);
        break;

    case Resource::Kind::Tile:
        res.url = util::mapbox::normalizeTileURL(apiBaseURL, resource.url, accessToken);
        break;
    }

    return std::make_unique<OnlineFileRequest>(std::move(res), std::move(callback), *impl);
}

OnlineFileRequest::OnlineFileRequest(Resource resource_, Callback callback_, OnlineFileSource::Impl& impl_)
    : impl(impl_),
      resource(std::move(resource_)),
      callback(std::move(callback_)) {
    impl.add(this);

    // Force an immediate first request if we don't have an expiration time.
    if (resource.priorExpires) {
        schedule(resource.priorExpires);
    } else {
        schedule(util::now());
    }
}

OnlineFileRequest::~OnlineFileRequest() {
    Log::Error(Event::Setup, "Destructeur OnlineFileRequest %p",this);
    impl.remove(this);
}

Timestamp interpolateExpiration(const Timestamp& current,
                                optional<Timestamp> prior,
                                bool& expired) {
    auto now = util::now();
    if (current > now) {
        return current;
    }
    
    if (current == now) {
        return now + util::CLOCK_MAPPY_TRAFFIC_RETRY_TIMEOUT;
    }

    if (!bool(prior)) {
        expired = true;
        return current;
    }

    // Expiring date is going backwards,
    // fallback to exponential backoff.
    if (current < *prior) {
        expired = true;
        return current;
    }

    auto delta = current - *prior;

    // Server is serving the same expired resource
    // over and over, fallback to exponential backoff.
    if (delta == Duration::zero()) {
        expired = true;
        return current;
    }

    // Assume that either the client or server clock is wrong and
    // try to interpolate a valid expiration date (from the client POV)
    // observing a minimum timeout.
    return now + std::max<Seconds>(delta, util::CLOCK_SKEW_RETRY_TIMEOUT);
}

void OnlineFileRequest::schedule(optional<Timestamp> expires) {
    if (impl.isPending(this) || impl.isActive(this)) {
        // There's already a request in progress; don't start another one.
        return;
    }

    // If we're not being asked for a forced refresh, calculate a timeout that depends on how many
    // consecutive errors we've encountered, and on the expiration time, if present.
    
  //  Duration errorRetryTimeout = http::errorRetryTimeout(failedRequestReason, failedRequests, retryAfter);
    
   // Duration expirationTimeout = http::expirationTimeout(expires, expiredRequests);
    
   // Log::Error(Event::Setup, "errorRetryTimeout %i",std::chrono::duration_cast<std::chrono::nanoseconds>(errorRetryTimeout));

     //Log::Error(Event::Setup, "expirationTimeout %i", std::chrono::duration_cast<std::chrono::nanoseconds>(expirationTimeout));
    
    //Log::Error(Event::Setup, "failedReq %i",failedRequests);
    //Log::Error(Event::Setup, "expiredReq %i",expiredRequests);
    
   // Log::Error(Event::Setup, "expirationTimeout %i", std::chrono::duration_cast<std::chrono::nanoseconds>(expirationTimeout));
    
    Duration timeout = std::min(
                            http::errorRetryTimeout(failedRequestReason, failedRequests, retryAfter),
                            http::expirationTimeout(expires, expiredRequests));
    
   // Log::Error(Event::Setup, "Duration %i" ,std::chrono::duration_cast<std::chrono::nanoseconds>(timeout));
   // Log::Error(Event::Setup, "Duration max %i",std::chrono::duration_cast<std::chrono::nanoseconds>(Duration::max()));

    if (timeout == Duration::max()) {
        return;
    }

    // Emulate a Connection error when the Offline mode is forced with
    // a really long timeout. The request will get re-triggered when
    // the NetworkStatus is set back to Online.
    if (NetworkStatus::Get() == NetworkStatus::Status::Offline) {
        failedRequestReason = Response::Error::Reason::Connection;
        failedRequests = 1;
         Log::Error(Event::Setup, "offline");
        timeout = Duration::max();
    }

   // Log::Error(Event::Setup, "timer start %i", std::chrono::duration_cast<std::chrono::nanoseconds>(timeout));
    timer.start(timeout, Duration::zero(), [&] {
     //   Log::Error(Event::Setup, "Queue Request");
            impl.activateOrQueueRequest(this);
    });
}

void OnlineFileRequest::completed(Response response) {
    // If we didn't get various caching headers in the response, continue using the
    // previous values. Otherwise, update the previous values to the new values.

    if (!response.modified) {
        response.modified = resource.priorModified;
    } else {
        resource.priorModified = response.modified;
    }

    bool isExpired = false;

    if (response.expires) {
        auto prior = resource.priorExpires;
        resource.priorExpires = response.expires;
        response.expires = interpolateExpiration(*response.expires, prior, isExpired);
    }

    if (isExpired) {
        expiredRequests++;
    } else {
        expiredRequests = 0;
    }

    if (!response.etag) {
        response.etag = resource.priorEtag;
    } else {
        resource.priorEtag = response.etag;
    }

    if (response.error) {
        failedRequests++;
        failedRequestReason = response.error->reason;
        retryAfter = response.error->retryAfter;
    } else {
        failedRequests = 0;
        failedRequestReason = Response::Error::Reason::Success;
    }
    
    Log::Error(Event::Setup, "OnlineFileReques::completed: error %i , failed % i , expiredRequests %i", failedRequestReason,failedRequests,expiredRequests);

    schedule(response.expires);

    // Calling the callback may result in `this` being deleted. It needs to be done last,
    // and needs to make a local copy of the callback to ensure that it remains valid for
    // the duration of the call.
    auto callback_ = callback;
    callback_(response);
}

void OnlineFileRequest::networkIsReachableAgain() {
    // We need all requests to fail at least once before we are going to start retrying
    // them, and we only immediately restart request that failed due to connection issues.
    if (failedRequestReason == Response::Error::Reason::Connection) {
        schedule(util::now());
    }
}

} // namespace mbgl
