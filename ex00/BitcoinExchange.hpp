#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <limits>

class BitcoinExchange
{
private:
    std::map<std::string, double> _rates;

    std::string trim(const std::string &s) const;
    bool isValidDate(const std::string &date) const;
    double parseValue(const std::string &valueStr) const;
    double getRateForDate(const std::string &date) const;

public:
    BitcoinExchange();
    BitcoinExchange(const BitcoinExchange &other);
    BitcoinExchange &operator=(const BitcoinExchange &other);
    ~BitcoinExchange();

    void loadDatabase(const std::string &csvPath);
    void processInputFile(const std::string &inputPath) const;
};

#endif