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

    const std::string injected_value   = "3\noutside_software: changed # comment\\\"\\\\\t";
    const std::string injected_key     = "outside:\nsoftware";
    (*document)["software"]["node_id"] = injected_value;
    (*document)[injected_key]          = injected_value;

    const auto emitted = yaml.emit(*document);
    assert(emitted.has_value());

    const auto reparsed = yaml.parse(*emitted);
    assert(reparsed.has_value());
    const auto* software = findMappingValue(*reparsed, "software");
    assert(software != nullptr);
    assertScalarEquals(findMappingValue(*software, "node_id"), injected_value);
    assertScalarEquals(findMappingValue(*reparsed, injected_key), injected_value);
    assert(findMappingValue(*reparsed, "outside_software") == nullptr);
}
