// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 LIBSYSTEMINFO_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// LIBSYSTEMINFO_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef LIBSYSTEMINFO_EXPORTS
#define LIBSYSTEMINFO_API __declspec(dllexport)
#else
#define LIBSYSTEMINFO_API __declspec(dllimport)
#endif
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <string>
#include <vector>
#include <map>
#include <json.hpp>
#pragma comment(lib, "wbemuuid.lib")
// 此类是从 dll 导出的
#define _MUTIPLE_INFO 1
#define _DEBUG_WIN_ (0)

class LIBSYSTEMINFO_API WindowsDeviceInfo {
public:
    WindowsDeviceInfo();
    virtual ~WindowsDeviceInfo();

    std::string GetCpuId();
    std::vector<std::string> GetCpuIds();
    std::string GetCpuName();
    std::vector<std::string> GetCpuNames();
    std::string GetDiskInfo(std::wstring property);
    std::vector<std::string> GetDiskInfos(std::wstring property);
    std::string GetDiskSerialNumber();
    std::vector<std::map<std::string, std::string>> GetDiskSerialNumbers();
    std::string GetDiskName();
    std::vector<std::string> GetDiskNames();
    std::string GetDiskSize();
    std::vector<std::map<std::string, std::string>> GetDiskSizes();
    std::string GetDiskFSpace();
    std::vector<std::string> GetDiskFSpaces();

    std::string GetGPUName();
    std::vector<std::string> GetGPUNames();
    std::string GetGPUSerialNumber();
    std::vector<std::string> GetGPUSerialNumbers();
    std::vector<std::map<std::string, std::string>> GetDetailedNetworkInfo();
    std::string GetMACAddress();
    std::vector<std::string> GetMACAddresses();
    std::string GetOperatingSystem();
    std::string GetMemorySize();
    std::vector<std::map<std::string, std::string>> GetMemorySizes();
    std::string GetMemoryName();
    std::vector<std::string> GetMemoryNames();
    std::string GetMainboard();
    std::vector<std::map<std::string, std::string>> GetMainboards();
    std::vector<std::string> GetDiskPartitions();

    nlohmann::json GetAllDeviceInfoAsJson();

private:
    IWbemLocator* pLocator = NULL;
    IWbemServices* pServices = NULL;
    std::map<std::string, std::wstring> diskInfoMap;

    bool InitializeCOM();
    void UninitializeCOM();
    bool ConnectToWMI();
    void DisconnectFromWMI();
    std::string ConvertBSTRToUTF8(const BSTR bstr);
    std::string QueryWMI(const std::wstring& query, const std::wstring& property);
    std::vector<std::string> QueryWMIMultiple(const std::wstring& query, const std::wstring& property);
};
extern "C" {
    LIBSYSTEMINFO_API WindowsDeviceInfo* CreateWindowsDeviceInfo();
    LIBSYSTEMINFO_API void DeleteWindowsDeviceInfo(WindowsDeviceInfo* obj);
    LIBSYSTEMINFO_API char* CallGetAllDeviceInfoAsJson(WindowsDeviceInfo* instance);
    LIBSYSTEMINFO_API char* CallGetAllDeviceInfoAsJsonBase64(WindowsDeviceInfo* instance);
    LIBSYSTEMINFO_API void WriteSystemInfoToEnv(WindowsDeviceInfo* instance);
    LIBSYSTEMINFO_API char* ReadBase64SystemInfoFromEnv();
    LIBSYSTEMINFO_API char* ReadAndDecodeSystemInfoFromEnv();

}
//extern LIBSYSTEMINFO_API int nLibSystemInfo;

//LIBSYSTEMINFO_API int fnLibSystemInfo(void);
