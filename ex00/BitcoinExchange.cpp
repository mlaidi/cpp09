#include "BitcoinExchange.hpp"
#include <sstream>

BitcoinExchange::BitcoinExchange() {}

BitcoinExchange::BitcoinExchange(const BitcoinExchange &other)
    : _rates(other._rates)
{
}

BitcoinExchange &BitcoinExchange::operator=(const BitcoinExchange &other)
{
    if (this != &other)
        _rates = other._rates;
    return *this;
}

BitcoinExchange::~BitcoinExchange() {}


std::string BitcoinExchange::trim(const std::string &s) const
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool BitcoinExchange::isValidDate(const std::string &date) const
{
    if (date.size() != 10)
        return false;
    if (date[4] != '-' || date[7] != '-')
        return false;
    for (size_t i = 0; i < 10; ++i)
    {
        if (i == 4 || i == 7)
            continue;
        if (!std::isdigit(date[i]))
            return false;
    }
    int year  = std::atoi(date.substr(0, 4).c_str());
    int month = std::atoi(date.substr(5, 2).c_str());
    int day   = std::atoi(date.substr(8, 2).c_str());

    if (month < 1 || month > 12)
        return false;
    if (day < 1 || day > 31)
        return false;

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        daysInMonth[1] = 29;
    if (day > daysInMonth[month - 1])
        return false;

    return true;
}

double BitcoinExchange::parseValue(const std::string &valueStr) const
{
    std::string s = trim(valueStr);
    if (s.empty())
        throw std::runtime_error("bad value");

    size_t start = 0;
    if (s[0] == '-' || s[0] == '+')
        start = 1;
    bool hasDot = false;
    for (size_t i = start; i < s.size(); ++i)
    {
        if (s[i] == '.')
        {
            if (hasDot)
                throw std::runtime_error("bad value");
            hasDot = true;
        }
        else if (!std::isdigit(s[i]))
            throw std::runtime_error("bad value");
    }
    if (start == s.size())
        throw std::runtime_error("bad value");

    double val = std::strtod(s.c_str(), NULL);
    return val;
}

double BitcoinExchange::getRateForDate(const std::string &date) const
{
    if (_rates.empty())
        throw std::runtime_error("database is empty");

    std::map<std::string, double>::const_iterator it = _rates.lower_bound(date);

    if (it != _rates.end() && it->first == date)
        return it->second;

    if (it == _rates.begin())
        throw std::runtime_error("date too early, no lower rate available");

    --it;
    return it->second;
}


void BitcoinExchange::loadDatabase(const std::string &csvPath)
{
    std::ifstream file(csvPath.c_str());
    if (!file.is_open())
        throw std::runtime_error("could not open database file.");

    std::string line;
    bool firstLine = true;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        if (firstLine)
        {
            firstLine = false;
            if (line == "date,exchange_rate")
                continue;
        }

        size_t comma = line.find(',');
        if (comma == std::string::npos)
            continue;

        std::string date  = trim(line.substr(0, comma));
        std::string rateS = trim(line.substr(comma + 1));

        if (!isValidDate(date))
            continue;

        double rate = std::strtod(rateS.c_str(), NULL);
        _rates[date] = rate;
    }
}

void BitcoinExchange::processInputFile(const std::string &inputPath) const
{
    std::ifstream file(inputPath.c_str());
    if (!file.is_open())
    {
        std::cerr << "Error: could not open file." << std::endl;
        return;
    }

    std::string line;
    bool firstLine = true;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        if (firstLine)
        {
            firstLine = false;
            if (line == "date | value")
                continue;
        }

        size_t sep = line.find(" | ");
        if (sep == std::string::npos)
        {
            std::cout << "Error: bad input => " << line << std::endl;
            continue;
        }

        std::string date     = trim(line.substr(0, sep));
        std::string valueStr = trim(line.substr(sep + 3));

        if (!isValidDate(date))
        {
            std::cout << "Error: bad input => " << line << std::endl;
            continue;
        }

        double value;
        try
        {
            value = parseValue(valueStr);
        }
        catch (std::exception &)
        {
            std::cout << "Error: bad input => " << line << std::endl;
            continue;
        }

        if (value < 0)
        {
            std::cout << "Error: not a positive number." << std::endl;
            continue;
        }
        if (value > 1000)
        {
            std::cout << "Error: too large a number." << std::endl;
            continue;
        }

        double rate;
        try
        {
            rate = getRateForDate(date);
        }
        catch (std::exception &e)
        {
            std::cout << "Error: " << e.what() << std::endl;
            continue;
        }

        std::cout << date << " => " << value << " = " << value * rate << std::endl;
    }
}
