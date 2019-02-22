
#include <mbgl/test/util.hpp>

#include <mbgl/text/tagged_string.hpp>

using namespace mbgl;

TEST(TaggedString, Trim) {
    TaggedString basic(u" \t\ntrim that and not this  \n\t", SectionOptions(1.0f, 0));
    basic.trim();
    EXPECT_EQ(basic.rawText(), u"trim that and not this");
    
    TaggedString twoSections;
    twoSections.addSection(u" \t\ntrim that", 1.5f, 1);
    twoSections.addSection(u" and not this  \n\t", 0.5f, 2);
    
    twoSections.trim();
    EXPECT_EQ(twoSections.rawText(), u"trim that and not this");
    
    TaggedString empty(u"\n\t\v \r  \t\n", SectionOptions(1.0f, 0));
    empty.trim();
    EXPECT_EQ(empty.rawText(), u"");
    
    TaggedString noTrim(u"no trim!", SectionOptions(1.0f, 0));
    noTrim.trim();
    EXPECT_EQ(noTrim.rawText(), u"no trim!");
}
