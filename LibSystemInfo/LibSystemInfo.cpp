// LibSystemInfo.cpp : 定义 DLL 的导出函数。
//

#include "framework.h"
#include "LibSystemInfo.h"
#include "JsonStringUtils.h"

#if defined(_WIN32) || defined(_WIN64)
#include <algorithm>  // Include for std::sort
#include <comdef.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
 
WindowsDeviceInfo::WindowsDeviceInfo()
{
    if (InitializeCOM())
    {
        ConnectToWMI();
    }
}

WindowsDeviceInfo::~WindowsDeviceInfo()
{
    DisconnectFromWMI();
    UninitializeCOM();
}

bool WindowsDeviceInfo::InitializeCOM()
{
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        return false;
    }
    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    return SUCCEEDED(hres);
}

void WindowsDeviceInfo::UninitializeCOM()
{
    CoUninitialize();
}

bool WindowsDeviceInfo::ConnectToWMI()
{
    HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, reinterpret_cast<void**>(&pLocator));
    if (FAILED(hres))
    {
        return false;
    }
    hres = pLocator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pServices);
    return SUCCEEDED(hres);
}

void WindowsDeviceInfo::DisconnectFromWMI()
{
    if (pServices)
    {
        pServices->Release();
    }
    if (pLocator)
    {
        pLocator->Release();
    }
}

std::string WindowsDeviceInfo::GetCpuId()
{
    return QueryWMI(L"SELECT ProcessorId FROM Win32_Processor", L"ProcessorId");
}
std::vector<std::string> WindowsDeviceInfo::GetCpuIds()
{
    return QueryWMIMultiple(L"SELECT ProcessorId FROM Win32_Processor", L"ProcessorId");
}

std::string WindowsDeviceInfo::GetCpuName()
{
    return QueryWMI(L"SELECT Name FROM Win32_Processor", L"Name");
}

std::vector<std::string> WindowsDeviceInfo::GetCpuNames()
{
    return QueryWMIMultiple(L"SELECT Name FROM Win32_Processor", L"Name");
}

std::string WindowsDeviceInfo::GetDiskInfo(std::wstring property)
{
    std::string result = QueryWMI(L"SELECT " + property + L" FROM Win32_DiskDrive", property);
    return result;
}

std::vector<std::string> WindowsDeviceInfo::GetDiskInfos(std::wstring property)
{
    std::vector<std::string> result = QueryWMIMultiple(L"SELECT " + property + L" FROM Win32_DiskDrive", property);
    std::string propertyStr(property.begin(), property.end());
    return result;
}

std::string WindowsDeviceInfo::GetDiskSerialNumber()
{

    return GetDiskInfo(diskInfoMap["SerialNumber"]);
}
std::vector<std::map<std::string, std::string>> WindowsDeviceInfo::GetDiskSerialNumbers()
{
    std::vector<std::string> serialNumbers = GetDiskInfos(diskInfoMap["SerialNumber"]);
    std::vector<std::string> Names = GetDiskInfos(diskInfoMap["Name"]);
    std::vector<std::map<std::string, std::string>> diskInfo;
    for (int i = 0; i<serialNumbers.size(); i++) {
        std::map<std::string, std::string> temp;
        temp["SN"] = serialNumbers[i];
        temp["Name"] = Names[i];
        diskInfo.push_back(temp);
    }
    return diskInfo;
    // return GetDiskInfo(diskInfoMap["SerialNumber"]);
}

std::string WindowsDeviceInfo::GetDiskName()
{

    return GetDiskInfo(diskInfoMap["Name"]);
}
std::vector<std::string> WindowsDeviceInfo::GetDiskNames()
{

    return GetDiskInfos(diskInfoMap["Name"]);
}

std::string WindowsDeviceInfo::GetDiskSize()
{

    return GetDiskInfo(diskInfoMap["Size"]);
}
std::vector<std::map<std::string, std::string>> WindowsDeviceInfo::GetDiskSizes()
{
    // 获取磁盘大小和可用大小信息
    std::vector<std::string> sizes = GetDiskInfos(diskInfoMap["Size"]);
    std::vector<std::string> availableSizes = GetDiskInfos(diskInfoMap["FreeSpace"]);

    // 声明存储磁盘信息的向量
    std::vector<std::map<std::string, std::string>> diskInfo;

    // 确保 sizes 和 availableSizes 的长度相同
    size_t diskCount = sizes.size();

    for (size_t i = 0; i < diskCount; ++i) {
        std::map<std::string, std::string> temp;

        // 添加大小和可用空间
        temp["Size"] = sizes[i];
        temp["FreeSpace"] = availableSizes[i];

        try {
            // 计算并添加已用空间
            unsigned long long size = std::stoull(sizes[i]);
            unsigned long long availableSize = std::stoull(availableSizes[i]);
            temp["UsedSpace"] = std::to_string(size - availableSize);
        }
        catch (const std::invalid_argument& e) {
            // 处理无效参数异常
            temp["UsedSpace"] = e.what();
        }
        catch (const std::out_of_range& e) {
            // 处理范围超出异常
            temp["UsedSpace"] = e.what();
        }

        // 仅在有效数据情况下将结果插入到磁盘信息向量中
        diskInfo.push_back(temp);
    }

    return diskInfo;
}


std::string WindowsDeviceInfo::GetDiskFSpace()
{

    return GetDiskInfo(diskInfoMap["FreeSpace"]);
}
std::vector<std::string> WindowsDeviceInfo::GetDiskFSpaces()
{

    return GetDiskInfos(diskInfoMap["FreeSpace"]);
}
std::vector<std::string> WindowsDeviceInfo::GetDiskPartitions()
{
    return QueryWMIMultiple(L"SELECT Name FROM Win32_DiskPartition", L"Name");
}

// std::vector<std::string> WindowsDeviceInfo::GetDiskSizes() {
//     return QueryWMIMultiple(L"SELECT Size FROM Win32_DiskDrive", L"Size");
// }

std::string WindowsDeviceInfo::GetGPUName()
{
    std::string gpuName = QueryWMI(L"SELECT Name FROM Win32_VideoController", L"Name");
    return gpuName;
}
std::vector<std::string> WindowsDeviceInfo::GetGPUNames()
{
    return QueryWMIMultiple(L"SELECT Name FROM Win32_VideoController", L"Name");
}

std::string WindowsDeviceInfo::GetGPUSerialNumber()
{
    // 注意：通常情况下，WMI 不直接提供 GPU 序列号。这个查询可能需要厂商特定的扩展。
    return QueryWMI(L"SELECT PNPDeviceID FROM Win32_VideoController", L"PNPDeviceID");
    // return QueryWMI(L"SELECT Tag FROM Win32_VideoController", L"Tag");
}
std::vector<std::string> WindowsDeviceInfo::GetGPUSerialNumbers()
{
    return QueryWMIMultiple(L"SELECT PNPDeviceID FROM Win32_VideoController", L"PNPDeviceID");
}

std::string WindowsDeviceInfo::GetMACAddress()
{
    return QueryWMI(L"SELECT MACAddress FROM Win32_NetworkAdapter WHERE MACAddress IS NOT NULL", L"MACAddress");
}



std::vector<std::map<std::string, std::string>> WindowsDeviceInfo::GetDetailedNetworkInfo() {
    std::vector<std::map<std::string, std::string>> detailedInfo;

    // 获取所有必要的信息
    std::vector<std::string> macAddresses = QueryWMIMultiple(L"SELECT MACAddress FROM Win32_NetworkAdapter", L"MACAddress");
    std::vector<std::string> names = QueryWMIMultiple(L"SELECT Name FROM Win32_NetworkAdapter", L"Name");
    std::vector<std::string> descriptions = QueryWMIMultiple(L"SELECT Description FROM Win32_NetworkAdapter", L"Description");

    size_t numAdapters = macAddresses.size();
    for (size_t i = 0; i < numAdapters; ++i) {
        std::map<std::string, std::string> temp;
        temp["MACAddress"] = i < macAddresses.size() ? macAddresses[i] : "Unknown";
        temp["Name"] = i < names.size() ? names[i] : "Unknown";
        temp["Description"] = i < descriptions.size() ? descriptions[i] : "Unknown";

        if (temp["MACAddress"].find("Unknown") != std::string::npos) {
            continue; // Skip this iteration if MACAddress is Unknown
        }

        // 判断类型
        std::string desc = temp["Description"];
        if (desc.find("VMware") != std::string::npos || desc.find("VirtualBox") != std::string::npos || desc.find("Virtual") != std::string::npos) {
            temp["Type"] = "Virtual Machine";
        }
        else if (desc.find("Bluetooth") != std::string::npos) {
            temp["Type"] = "Bluetooth";
        }
        else if (desc.find("Wireless") != std::string::npos || desc.find("Wi-Fi") != std::string::npos || desc.find("WLAN") != std::string::npos) {
            temp["Type"] = "WLAN";
        }
        else if (desc.find("Ethernet") != std::string::npos || desc.find("Realtek") != std::string::npos || desc.find("Intel(R)") != std::string::npos) {
            temp["Type"] = "Physical";
        }
        else {
            temp["Type"] = "others";
        }

        detailedInfo.push_back(temp);
    }

    // 对详细信息进行排序，使 Physical 和 WLAN 类型的设备排在前面
    std::sort(detailedInfo.begin(), detailedInfo.end(), [](const std::map<std::string, std::string>& a, const std::map<std::string, std::string>& b) {
        const std::string& typeA = a.at("Type");
        const std::string& typeB = b.at("Type");
        if ((typeA == "Physical" || typeA == "WLAN") && (typeB != "Physical" && typeB != "WLAN")) {
            return true; // A 在 B 前
        }
        else if ((typeB == "Physical" || typeB == "WLAN") && (typeA != "Physical" && typeA != "WLAN")) {
            return false; // B 在 A 前
        }
        return typeA < typeB; // 否则按类型字母顺序排序
        });

    // 输出信息以便检查效果
    // for (const auto& adapter : detailedInfo) {
    //     std::cout << "Name: " << adapter.at("Name")
    //               << ", MAC: " << adapter.at("MACAddress")
    //               << ", Description: " << adapter.at("Description")
    //               << ", Type: " << adapter.at("Type") << std::endl;
    // }

    return detailedInfo;
}

std::string WindowsDeviceInfo::GetOperatingSystem()
{
    // return QueryWMI(L"SELECT Caption FROM Win32_OperatingSystem", L"Caption");
#if defined(_WIN32) || defined(_WIN64)
    return "Windows";
#endif
}

std::string WindowsDeviceInfo::GetMemorySize()
{
    // 获取总内存
    std::string total = QueryWMI(L"SELECT TotalPhysicalMemory FROM Win32_ComputerSystem", L"TotalPhysicalMemory");

    // 获取可用内存
    std::string availableBytes = QueryWMI(L"SELECT FreePhysicalMemory FROM Win32_OperatingSystem", L"FreePhysicalMemory");
    // 计算已用内存
    unsigned long long totalBytes = std::stoull(total);
    unsigned long long availableBytesValue = std::stoull(availableBytes) * 1024; // kB 转换为 Bytes
    unsigned long long usedBytes = totalBytes - availableBytesValue;

    // 转换为字符串
    std::string used = std::to_string(usedBytes);
    std::string totalStr = std::to_string(totalBytes);
    std::string availableStr = std::to_string(availableBytesValue);
    return "total: " + totalStr + " used: " + used + " available: " + availableStr;
    // return QueryWMI(L"SELECT Capacity FROM Win32_PhysicalMemory", L"Capacity");
}

std::vector<std::map<std::string, std::string>> WindowsDeviceInfo::GetMemorySizes()
{
    std::vector<std::map<std::string, std::string>> memoryInfo;
    std::vector<std::string> sizes = QueryWMIMultiple(L"SELECT Capacity FROM Win32_PhysicalMemory", L"Capacity");
    std::vector<std::string> availableSizes = QueryWMIMultiple(L"SELECT FreePhysicalMemory FROM Win32_OperatingSystem", L"FreePhysicalMemory");

    for (size_t i = 0; i < sizes.size(); i++) {
        std::map<std::string, std::string> temp;

        // 添加大小和可用空间
        temp["Size"] = sizes[i];
        temp["FreeSpace"] = availableSizes[i];

        try {
            // 计算并添加已用空间
            unsigned long long size = std::stoull(sizes[i]);
            unsigned long long availableSize = std::stoull(availableSizes[i]);
            temp["UsedSpace"] = std::to_string(size - availableSize);
        }
        catch (const std::invalid_argument& e) {
            // 处理无效参数异常
            temp["UsedSpace"] = e.what();
        }
        catch (const std::out_of_range& e) {
            // 处理范围超出异常
            temp["UsedSpace"] = e.what();
        }

        // 仅在有效数据情况下将结果插入到磁盘信息向量中
        memoryInfo.push_back(temp);
    }

    return memoryInfo;
}
std::string WindowsDeviceInfo::GetMemoryName()
{
    return QueryWMI(L"SELECT Manufacturer, manufacturer FROM Win32_PhysicalMemory", L"manufacturer");
}
std::vector<std::string> WindowsDeviceInfo::GetMemoryNames()
{
    return QueryWMIMultiple(L"SELECT Manufacturer FROM Win32_PhysicalMemory", L"Manufacturer");
}

std::string WindowsDeviceInfo::GetMainboard()
{
    std::map<std::string, std::string> result;

    // Query manufacturer
    std::string manufacturer = QueryWMI(L"SELECT Manufacturer FROM Win32_BaseBoard", L"Manufacturer");
    if (manufacturer.empty()) {
        manufacturer = "Unknown or Error Fetching Manufacturer";
    }
    result["Manufacturer"] = manufacturer;

    // Query name
    std::string name = QueryWMI(L"SELECT Name FROM Win32_BaseBoard", L"Name");
    if (name.empty()) {
        name = "Unknown or Error Fetching Name";
    }

    result["Name"] = name;


    // Query product
    std::string product = QueryWMI(L"SELECT Product FROM Win32_BaseBoard", L"Product");
    if (product.empty()) {
        product = "Unknown or Error Fetching Product";
    }
    result["Product"] = product;
    // system("pause");
    return result["Name"] + " " + result["Manufacturer"] + " " + result["Product"];
}

std::vector<std::map<std::string, std::string>> WindowsDeviceInfo::GetMainboards()
{
    std::vector<std::map<std::string, std::string>> mainboards;
    std::vector<std::string> manufacturers = QueryWMIMultiple(L"SELECT Manufacturer FROM Win32_BaseBoard", L"Manufacturer");
    std::vector<std::string> names = QueryWMIMultiple(L"SELECT Name FROM Win32_BaseBoard", L"Name");
    std::vector<std::string> products = QueryWMIMultiple(L"SELECT Product FROM Win32_BaseBoard", L"Product");

    for (size_t i = 0; i < manufacturers.size(); i++) {
        std::map<std::string, std::string> temp;
        temp["Manufacturer"] = manufacturers[i];
        temp["Name"] = names[i];
        temp["Product"] = products[i];
        mainboards.push_back(temp);
    }

    return mainboards;
}

std::string WindowsDeviceInfo::ConvertBSTRToUTF8(const BSTR bstr) {
    if (!bstr)
        return "Unknown or Invalid Property Value";  // 返回默认值而非空字符串

    // 计算需要的长度
    int length = WideCharToMultiByte(CP_UTF8, 0, bstr, -1, nullptr, 0, nullptr, nullptr);
    if (length <= 0) return "Conversion Failed";

    // 创建字符串并转换
    std::string utf8_string(length - 1, 0); // 减去空终止符
    WideCharToMultiByte(CP_UTF8, 0, bstr, -1, &utf8_string[0], length, nullptr, nullptr);

    return utf8_string;
}

std::string WindowsDeviceInfo::QueryWMI(const std::wstring& query, const std::wstring& property)
{
    // 查询处理器ID
    IEnumWbemClassObject* pEnumerator = NULL;
    HRESULT hres = pServices->ExecQuery(
        bstr_t("WQL"),
        _bstr_t(query.c_str()).GetBSTR(), // 正确转换为BSTR
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        return "Query Failed";
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    std::string result = "Unknown";
    if (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (uReturn != 0)
        {
            VARIANT vtProp;
            hr = pclsObj->Get(_bstr_t(property.c_str()), 0, &vtProp, 0, 0);
            if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && vtProp.bstrVal)
            {
                result = ConvertBSTRToUTF8(vtProp.bstrVal);
                VariantClear(&vtProp);
            }
            pclsObj->Release();
        }
        pEnumerator->Release();
    }
    return result;
}

std::vector<std::string> WindowsDeviceInfo::QueryWMIMultiple(const std::wstring& query, const std::wstring& property)
{
    std::vector<std::string> results;

    IEnumWbemClassObject* pEnumerator = NULL;
    HRESULT hres = pServices->ExecQuery(
        bstr_t("WQL"),
        _bstr_t(query.c_str()).GetBSTR(), // 正确转换为 BSTR
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        results.push_back("Query Failed: " + std::to_string(hres));
        return results;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(_bstr_t(property.c_str()), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && vtProp.bstrVal != NULL)
        {
            // 使用 UTF-8 转换函数
            results.push_back(ConvertBSTRToUTF8(vtProp.bstrVal));
        }
        else
        {
            results.push_back("Unknown or Invalid Property Value");
        }
        VariantClear(&vtProp);
        if (pclsObj)
            pclsObj->Release();
    }
    if (pEnumerator)
        pEnumerator->Release();

    return results;
}

nlohmann::json WindowsDeviceInfo::GetAllDeviceInfoAsJson()
{
    nlohmann::json deviceInfo;
#if defined(_MUTIPLE_INFO)
    try
    {
        deviceInfo["CPU_SN"] = GetCpuIds();
#if _DEBUG_WIN_
        std::cout << "CPU_SN" << std::endl;
#endif
        deviceInfo["CPU_Name"] = GetCpuNames();
#if _DEBUG_WIN_
        std::cout << "CPU_Name" << std::endl;
#endif
        deviceInfo["Disk_SN"] = GetDiskSerialNumbers();
#if _DEBUG_WIN_
        std::cout << "Disk_SN" << std::endl;
#endif
        deviceInfo["GPU_Name"] = GetGPUNames();
#if _DEBUG_WIN_
        std::cout << "GPU_Name" << std::endl;
#endif
        deviceInfo["GPU_SN"] = GetGPUSerialNumbers();
#if _DEBUG_WIN_
        std::cout << "GPU_SN" << std::endl;
#endif
        deviceInfo["MAC"] = GetDetailedNetworkInfo();
#if _DEBUG_WIN_
        std::cout << "MAC" << std::endl;
#endif
        deviceInfo["Mainboard"] = GetMainboards();
#if _DEBUG_WIN_
        std::cout << "Mainboard" << std::endl;
#endif
        deviceInfo["Operating_System"] = GetOperatingSystem();
#if _DEBUG_WIN_
        std::cout << "Operating_System" << std::endl;
#endif
        deviceInfo["Memory Size"] = GetMemorySizes();
#if _DEBUG_WIN_
        std::cout << "Memory Size" << std::endl;
#endif
        deviceInfo["Memory Name"] = GetMemoryNames();
#if _DEBUG_WIN_
        std::cout << "Memory Name" << std::endl;
#endif
        /* code */
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }


    // std::cout<<"damn"<<std::endl;
#else
    // 收集所有信息
    deviceInfo["CPU_SN"] = GetCpuId();
    deviceInfo["CPU Name"] = GetCpuName();
    deviceInfo["Disk Serial Number"] = GetDiskSerialNumber();
    deviceInfo["GPU Name"] = GetGPUName();
    // 无法获取
    // deviceInfo["GPU Serial Number"] = GetGPUSerialNumber();
    deviceInfo["MAC Address"] = GetMACAddress();
    // 会终止
    deviceInfo["Operating System"] = GetOperatingSystem();
    deviceInfo["Memory Size"] = GetMemorySize();
    // 为空
    // deviceInfo["Memory Name"] = GetMemoryName();
    deviceInfo["Mainboard"] = GetMainboard();
    // 会终止
    // deviceInfo["Disk Partitions"] = GetDiskPartitions();
    deviceInfo["Disk Sizes"] = GetDiskSizes();

#endif
    // system("pause");
    return deviceInfo;
}
LIBSYSTEMINFO_API WindowsDeviceInfo* CreateWindowsDeviceInfo() {
    return new WindowsDeviceInfo();
}

LIBSYSTEMINFO_API void DeleteWindowsDeviceInfo(WindowsDeviceInfo* instance) {
    delete instance;
}
LIBSYSTEMINFO_API char* CallGetAllDeviceInfoAsJson(WindowsDeviceInfo* instance) {
    nlohmann::json json_obj = instance->GetAllDeviceInfoAsJson();  // 填充 JSON 对象的逻辑
    std::string json_str = json_obj.dump(4);  // 将 JSON 对象转换为字符串
    return Sting2Chars(json_str);
    //char* c_str = (char*)malloc(json_str.size() + 1);  // 分配内存
    //std::copy(json_str.begin(), json_str.end(), c_str);  // 复制数据
    //c_str[json_str.size()] = '\0';  // 确保字符串以空字符结尾
    ////char* c_str = NULL; 
    //return c_str;  // 返回指针
}

LIBSYSTEMINFO_API char* CallGetAllDeviceInfoAsJsonBase64(WindowsDeviceInfo* instance) {
    nlohmann::json deviceInfo = instance->GetAllDeviceInfoAsJson();
    std::string json_str = Base64Encode(deviceInfo.dump(4));
    return Sting2Chars(json_str);
    //char* c_str = (char*)malloc(json_str.size() + 1);  // 分配内存
    //std::copy(json_str.begin(), json_str.end(), c_str);  // 复制数据
    //c_str[json_str.size()] = '\0';  // 确保字符串以空字符结尾
    //
    //return c_str;
}
LIBSYSTEMINFO_API void WriteSystemInfoToEnv(WindowsDeviceInfo* instance) {
    std::string base64Info = CallGetAllDeviceInfoAsJsonBase64(instance);
    WriteToEnvFile(base64Info);
}

LIBSYSTEMINFO_API char* ReadBase64SystemInfoFromEnv() {
    //std::map<std::string, std::string> envVars = ParseEnvFile(".env");
    //std::string json_str= envVars["HARDWARE_INFO"];
    std::string json_str = ReadBase64FromEnv(".env");
    return Sting2Chars(json_str);
    //char* c_str = (char*)malloc(json_str.size() + 1);  // 分配内存
    //std::copy(json_str.begin(), json_str.end(), c_str);  // 复制数据
    //c_str[json_str.size()] = '\0';  // 确保字符串以空字符结尾

    //return c_str;
}
LIBSYSTEMINFO_API char* ReadAndDecodeSystemInfoFromEnv() {
    std::string base64Info = ReadBase64SystemInfoFromEnv();
    return Sting2Chars(Base64Decode(base64Info));
}



#endif}
