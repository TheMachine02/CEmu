/*
 * Part of tivars_lib_cpp
 * (C) 2015-2016 Adrien 'Adriweb' Bertrand
 * https://github.com/adriweb/tivars_lib_cpp
 * License: MIT
 */

#include "../autoloader.h"

#include <QtCore/QFile>

#include <fstream>
#include <sstream>
#include <regex>

using namespace std;

namespace tivars
{

    std::unordered_map<uint, std::vector<std::string>> tokens_BytesToName;
    std::unordered_map<std::string, uint> tokens_NameToBytes;
    uchar lengthOfLongestTokenName;
    std::vector<uchar> firstByteOfTwoByteTokens;

    data_t TH_0x05::makeDataFromString(const string& str, const options_t& options)
    {
        (void)options;
        data_t data;

        if (tokens_BytesToName.empty()) {
            initTokens();
        }

        // two bytes reserved for the size. Filled later
        data.push_back(0); data.push_back(0);

        for (uint strCursorPos = 0; strCursorPos < str.length(); strCursorPos++)
        {
            for (uint currentLength = lengthOfLongestTokenName; currentLength > 0; currentLength--)
            {
                string currentSubString = str.substr(strCursorPos, currentLength);
                if (tokens_NameToBytes.count(currentSubString))
                {
                    uint tokenValue = tokens_NameToBytes[currentSubString];
                    if (tokenValue > 0xFF)
                    {
                        data.push_back((uchar)(tokenValue >> 8));
                    }
                    data.push_back((uchar)(tokenValue & 0xFF));
                    strCursorPos += currentLength - 1;
                    break;
                }
            }
        }
        uint actualDataLen = (uint) (data.size() - 2);
        data[0] = (uchar)(actualDataLen & 0xFF);
        data[1] = (uchar)((actualDataLen >> 8) & 0xFF);
        return data;
    }

    string TH_0x05::makeStringFromData(const data_t& data, const options_t& options)
    {
        if (data.size() < 2)
        {
            std::cerr << "Empty data array. Needs to contain at least 2 bytes (size fields)" << endl;
            return "";
        }

        if (tokens_BytesToName.empty()) {
            initTokens();
        }

        uint langIdx = (has_option(options, "lang") && options.at("lang") == LANG_FR) ? LANG_FR : LANG_EN;

        uint howManyBytes = (data[0] & 0xFF) + ((data[1] << 8) & 0xFF00);
        if (howManyBytes != data.size() - 2)
        {
            cerr << "[Warning] Token count (" << (data.size() - 2) << ") and size field (" << howManyBytes  << ") mismatch!";
        }

        uint errCount = 0;
        string str("");
        size_t dataSize = data.size();
        for (uint i = 2; i < howManyBytes + 2; i++)
        {
            uint currentToken = data[i];
            uint nextToken = (i < dataSize-1) ? data[i+1] : (uint)-1;
            uint bytesKey = currentToken;
            if (is_in_vector(firstByteOfTwoByteTokens, (uchar)currentToken))
            {
                if (nextToken == (uint)-1)
                {
                    cerr << "[Warning] Encountered an unfinished two-byte token! Setting the second byte to 0x00";
                    nextToken = 0x00;
                }
                bytesKey = nextToken + (currentToken << 8);
                i++;
            }
            if (tokens_BytesToName.find(bytesKey) != tokens_BytesToName.end())
            {
                str += tokens_BytesToName[bytesKey][langIdx];
            } else  {
                str += " [???] ";
                errCount++;
            }
        }

        if (errCount > 0)
        {
            cerr << "[Warning] " << errCount << " token(s) could not be detokenized (' [???] ' was used)!";
        }

        if (has_option(options, "prettify") && options.at("prettify") == 1)
        {
            str = regex_replace(str, regex("\\[?\\|?([a-z]+)\\]?"), "$1");
        }

        if (has_option(options, "reindent") && options.at("reindent") == 1)
        {
            str = reindentCodeString(str);
        }

        return str;
    }

    string TH_0x05::reindentCodeString(const string& str_orig, const options_t& options)
    {
        int lang;
        if (has_option(options, "lang"))
        {
            lang = options.at("lang");
        } else {
            lang = (str_orig.size() > 1 && str_orig[0] == '.' && ::isalpha(str_orig[1])) ? PRGMLANG_AXE : PRGMLANG_BASIC;
        }

        string str(str_orig);

        str = regex_replace(str, regex("([^\\s])(Del|Eff)Var "), "$1\n$2Var");

        vector<string> lines_tmp = explode(str, '\n');

        // Inplace-replace the appropriate ":" by new-line chars (ie, by inserting the split string in the lines_tmp array)
        for (uint16_t idx = 0; idx < (uint16_t)lines_tmp.size(); idx++)
        {
            const auto line = lines_tmp[idx];
            bool isWithinString = false;
            for (uint16_t strIdx = 0; strIdx < (uint16_t)line.size(); strIdx++)
            {
                const auto currChar = line.substr(strIdx, 1);
                if (currChar == ":" && !isWithinString)
                {
                    lines_tmp[idx] = line.substr(0, strIdx); // replace "old" line by lhs
                    lines_tmp.insert(lines_tmp.begin() + idx + 1, line.substr(strIdx + 1)); // inserting rhs
                    break;
                } else if (currChar == "\"") {
                    isWithinString = !isWithinString;
                } else if (currChar == "\n" || currChar == "→") {
                    isWithinString = false;
                }
            }
        }

        vector<pair<uint, string>> lines; // indent, text
        for (auto& line : lines_tmp)
        {
            lines.emplace_back(0, line);
        }

        vector<string> increaseIndentAfter   = { "If", "For", "While", "Repeat" };
        vector<string> decreaseIndentOfToken = { "Then", "Else", "End", "ElseIf", "EndIf", "End!If" };
        vector<string> closingTokens         = { "End", "EndIf", "End!If" };
        uint nextIndent = 0;
        string oldFirstCommand = "", firstCommand = "";
        for (uint key=0; key<lines.size(); key++)
        {
            auto lineData = lines[key];
            oldFirstCommand = firstCommand;

            string trimmedLine = trim(lineData.second);
            if (trimmedLine.length() > 0) {
                char* trimmedLine_c = (char*) trimmedLine.c_str();
                firstCommand = strtok(trimmedLine_c, " ");
                firstCommand = trim(firstCommand);
                trimmedLine = string(trimmedLine_c);
                trimmedLine_c = (char*) trimmedLine.c_str();
                if (firstCommand == trimmedLine)
                {
                    firstCommand = strtok(trimmedLine_c, "(");
                    firstCommand = trim(firstCommand);
                }
            } else {
                firstCommand = "";
            }

            lines[key].first = nextIndent;

            if (is_in_vector(increaseIndentAfter, firstCommand))
            {
                nextIndent++;
            }
            if (lines[key].first > 0 && is_in_vector(decreaseIndentOfToken, firstCommand))
            {
                lines[key].first--;
            }
            if (nextIndent > 0 && (is_in_vector(closingTokens, firstCommand) || (oldFirstCommand == "If" && firstCommand != "Then" && lang != PRGMLANG_AXE)))
            {
                nextIndent--;
            }
        }

        str = "";
        for (const auto& line : lines)
        {
            str += str_repeat(" ", line.first * 3) + line.second + '\n';
        }

        return str;
    }

    void TH_0x05::initTokens()
    {
        QFile inputFile(":/other/tivarslib/TypeHandlers/programs_tokens.csv");
        if (inputFile.open(QIODevice::ReadOnly))
        {
            vector<vector<string>> lines;
            QString csvFileStr = inputFile.readAll();

            ParseCSV(csvFileStr.toStdString(), lines);

            for (const auto& tokenInfo : lines)
            {
                uint bytes;
                if (tokenInfo[6] == "2") // number of bytes for the token
                {
                    if (!is_in_vector(firstByteOfTwoByteTokens, hexdec(tokenInfo[7])))
                    {
                        firstByteOfTwoByteTokens.push_back(hexdec(tokenInfo[7]));
                    }
                    bytes = hexdec(tokenInfo[8]) + (hexdec(tokenInfo[7]) << 8);
                } else {
                    bytes = hexdec(tokenInfo[7]);
                }
                tokens_BytesToName[bytes] = { tokenInfo[4], tokenInfo[5] }; // EN, FR
                tokens_NameToBytes[tokenInfo[4]] = bytes; // EN
                tokens_NameToBytes[tokenInfo[5]] = bytes; // FR
                uchar maxLenName = (uchar) max(tokenInfo[4].length(), tokenInfo[5].length());
                if (maxLenName > lengthOfLongestTokenName)
                {
                    lengthOfLongestTokenName = maxLenName;
                }
            }

            (inputFile.*(&QFile::close))(); // Compiler/Linker weirdness when in LTO (undefined symbol).
        } else {
            std::cerr << "Could not open the tokens csv file" << std::endl;
        }
    }

}
