#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <cassert>
#include <string>

#include "EmbedYAML/EmbedYAML.hpp"

namespace
{

const EmbedYAML::Node* findMappingValue(const EmbedYAML::Node& node, const std::string& key)
{
    const auto* entries = node.mapEntries();
    if (entries == nullptr)
        return nullptr;

    for (const auto& entry : *entries)
    {
        if (entry.key == key)
            return entry.value.get();
    }
    return nullptr;
}

void assertScalarEquals(const EmbedYAML::Node* node, const std::string& expected)
{
    assert(node != nullptr);
    const auto value = node->as<std::string>();
    assert(value.has_value());
    assert(*value == expected);
}

}  // namespace

int main()
{
    EmbedYAML::EmbedYAML yaml;
    auto                 document = yaml.parse("software:\n  node_id: 3\n");
    assert(document.has_value());

    const std::string injected_value            = "3\noutside_software: changed # comment\\\"\\\\\t";
    const std::string injected_key              = "outside:\nsoftware";
    const std::string unicode_controls          = "before\xC2\x80"
                                                  "middle\xC2\x85"
                                                  "after";
    const std::string unicode_key               = "key\xC2\x85"
                                                  "name";
    (*document)["software"]["node_id"]          = injected_value;
    (*document)[injected_key]                   = injected_value;
    (*document)["software"]["unicode_controls"] = unicode_controls;
    (*document)[unicode_key]                    = unicode_controls;

    const auto emitted = yaml.emit(*document);
    assert(emitted.has_value());
    assert(emitted->find("\\x80") != std::string::npos);
    assert(emitted->find("\\x85") != std::string::npos);

    const auto reparsed = yaml.parse(*emitted);
    assert(reparsed.has_value());
    const auto* software = findMappingValue(*reparsed, "software");
    assert(software != nullptr);
    assertScalarEquals(findMappingValue(*software, "node_id"), injected_value);
    assertScalarEquals(findMappingValue(*software, "unicode_controls"), unicode_controls);
    assertScalarEquals(findMappingValue(*reparsed, injected_key), injected_value);
    assertScalarEquals(findMappingValue(*reparsed, unicode_key), unicode_controls);
    assert(findMappingValue(*reparsed, "outside_software") == nullptr);
}
