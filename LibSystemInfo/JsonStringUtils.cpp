#include "JsonStringUtils.h"

// Base64 ���뺯��
std::string Base64Encode(const std::string& input) {

    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string output;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        char encode_char = base64_chars[(val >> (8 + valb)) & 0x3F];
        // std::cout << "Encoded character: " << encode_char 
        //           << " (from input char: " << c << " | val: " << val
        //           << " | valb: " << valb << ")" << std::endl;
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            output.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) output.push_back(base64_chars[((val << 8) >> valb) & 0x3F]);
    while (output.size() % 4) output.push_back('=');
    return output;
}
// Base64 ���뺯��
std::string Base64Decode(const std::string& input) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string output;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;

    int val = 0, valb = -8;  // valb initially less than 0
    for (unsigned char c : input) {
        // Skip padding characters
        if (c == '=') break;

        if (T[c] == -1) {
            std::cout << "Invalid character: " << c << std::endl; // Invalid character check
            break;
        }

        val = (val << 6) + T[c];
        valb += 6;

        while (valb >= 0) {
            // Decode the character and push to output
            char decoded_char = char((val >> valb) & 0xFF);
            output.push_back(decoded_char);
            // std::cout << "Decoded character: " << decoded_char 
            //           << " (from Base64 char: " << c << " | val: " << val
            //           << " | valb: " << valb << ")" << std::endl;
            valb -= 8;
        }
    }

    return output;
}

char* Sting2Chars(const std::string& str)
{
    char* c_str = (char*)malloc(str.size() + 1);  // �����ڴ�
    std::copy(str.begin(), str.end(), c_str);  // ��������
    c_str[str.size()] = '\0';  // ȷ���ַ����Կ��ַ���β
    //char* c_str = NULL; 
    return c_str;
}
// �� Base64 ����д�� .env �ļ�
void WriteToEnvFile(const std::string& base64String) {
    std::ofstream envFile(".env", std::ios::app);
    if (envFile.is_open()) {
        envFile << "HARDWARE_INFO=" << base64String << std::endl;
        envFile.close();
    }
    else {
        std::cerr << "Unable to open .env file for writing." << std::endl;
    }
}
// �� JSON ����д���ִ�г�������Ŀ¼�µ��ļ�
void WriteJsonToExecutableDir(const nlohmann::json& jsonData) {
    // ��ȡ��ǰ����Ŀ¼
#ifdef _WIN32
    char buffer[MAX_PATH];
    auto p_d = GetCurrentDirectoryA(MAX_PATH, buffer);
    if (p_d != 0) {
        std::string execDir = buffer;  // ��ǰ��ִ�г���Ŀ¼
        std::ofstream jsonFile(execDir + "/HardwareInfoData.json");

        if (jsonFile.is_open()) {
            jsonFile << jsonData.dump(4);  // ʹ�� 4 ��Ϊ������ʽ�� JSON
            jsonFile.close();
            std::cout << "JSON data written to: " << (execDir + "/HardwareInfoData.json") << std::endl;
        }
        else {
            std::cerr << "Unable to open data.json file for writing." << std::endl;
        }
    }
    else {
        std::cerr << "Error getting current working directory." << std::endl;
    }

#else
    char buffer[PATH_MAX];
    auto p_d = getcwd(buffer, sizeof(buffer));
    if (p_d != nullptr) {
        std::string execDir = buffer;  // ��ǰ��ִ�г���Ŀ¼
        std::ofstream jsonFile(execDir + "/HardwareInfoData.json");

        if (jsonFile.is_open()) {
            jsonFile << jsonData.dump(4);  // ʹ�� 4 ��Ϊ������ʽ�� JSON
            jsonFile.close();
            std::cout << "JSON data written to: " << (execDir + "/HardwareInfoData.json") << std::endl;
        }
        else {
            std::cerr << "Unable to open data.json file for writing." << std::endl;
        }
    }
    else {
        std::cerr << "Error getting current working directory." << std::endl;
    }
#endif    
}
// ����.env�ļ�������һ���������б�����map
std::map<std::string, std::string> ParseEnvFile(const std::string& envFilePath) {
    std::cout << "Parsed ENV Variablesfunction:" << std::endl;
    std::map<std::string, std::string> envMap;
    std::ifstream envFile(envFilePath);
    std::string line;

    while (std::getline(envFile, line)) {
        std::string::size_type delimiterPos = line.find("=");
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            envMap[key] = value;
        }
    }
    std::cout << "Parsed ENV Variablesfunction end" << std::endl;
    return envMap;
}
std::string CleanJsonString(const std::string& input) {
    std::string output = input;
    // �滻���з�Ϊ��׼ JSON �ַ�
    std::replace(output.begin(), output.end(), '\n', ' '); // �����滻Ϊ '\\n'
    // ���Խ����������������
    return output;
}
// ���� JSON ����
nlohmann::json CleanJson(nlohmann::json jsonObj) {
    for (auto& item : jsonObj.items()) {
        if (item.value().is_string()) {
            // ��ȡ��ǰ�ַ���
            std::string currentString = item.value();
            // �滻���з�Ϊ�ո����������
            currentString.erase(std::remove(currentString.begin(), currentString.end(), '\n'), currentString.end());
            // ���� JSON �е��ַ���ֵ
            jsonObj[item.key()] = currentString;
        }
        else if (item.value().is_array()) {
            // ��������飬�ݹ鴦�������е�ÿ��Ԫ��
            for (auto& element : item.value()) {
                if (element.is_string()) {
                    std::string currentString = element;
                    currentString.erase(std::remove(currentString.begin(), currentString.end(), '\n'), currentString.end());
                    element = currentString;
                }
            }
        }
        // �������Ӷ������������͵Ĵ���
    }
    return jsonObj;
}
