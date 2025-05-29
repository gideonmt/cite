#pragma once
#include "../include/citation.hpp"

class MLAFormatter : public CitationFormatter {
public:
    std::string format(const nlohmann::json& entry) const override;
};
