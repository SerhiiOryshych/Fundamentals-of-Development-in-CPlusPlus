#include "ini.h"
#include <string>
#include <iostream>

namespace Ini {
    Section &Document::AddSection(string name) {
        return sections[name];
    }

    const Section &Document::GetSection(const string &name) const {
        return sections.at(name);
    }

    size_t Document::SectionCount() const {
        return sections.size();
    }

    Document Load(istream &input) {
        Document document;
        string line, section;
        while (getline(input, line)) {
            if (line.empty()) {
                continue;
            }

            if (line[0] == '[' && line.back() == ']') {
                section = line.substr(1, line.length() - 2);
                document.AddSection(section);
                continue;
            }

            if (!section.empty()) {
                auto separator = line.find('=');
                if (separator == string::npos) {
                    continue;
                }

                auto key = line.substr(0, separator);
                auto value = line.substr(separator + 1, line.length() - separator - 1);
                if (key.empty() || value.empty()) {
                    continue;
                }

                document.AddSection(section)[key] = value;
            }
        }

        return document;
    }
}