#include <windows.h>
#include <initguid.h>
#include <slpublic.h>
#include <sliddefs.h>
#include <slerror.h>
#include <iostream>

#pragma comment(lib, "slc.lib")
#pragma comment(lib, "slwga.lib")

void Convert8ToW(const std::string &str8, std::wstring &strW);
std::wstring GetBiosLicenseKey(void);
std::wstring GetWindowsActivationKey(void);

//#################################################################################################
int main(void)
{
	const GUID guidAppId = WINDOWS_SLID;
	std::wstring str;

	// Call SLIsGenuineLocal() to determine if Windows is genuine and activated
	SL_GENUINE_STATE state = SL_GEN_STATE_LAST;
	if(SUCCEEDED(SLIsGenuineLocal(&guidAppId, &state, nullptr)))
	{
		str = L"Installation status: ";
		if(state == SL_GEN_STATE_IS_GENUINE)
			str += L"genuine";
		else if(state == SL_GEN_STATE_INVALID_LICENSE)
			str += L"invalid";
		else if(state == SL_GEN_STATE_TAMPERED)
			str += L"tampered";
		else if(state == SL_GEN_STATE_OFFLINE)
			str += L"offline";
		else if(state == SL_GEN_STATE_LAST)
			str += L"unchanged";
		else
			str += L"unknown";

		std::wcout << str << std::endl;
	}

	std::wcout << L"Firmware embedded key: " << GetBiosLicenseKey() << std::endl;
	std::wcout << L"Windows activation key: " << GetWindowsActivationKey() << std::endl;

	HSLC hSLC = nullptr;
	if(SUCCEEDED(SLOpen(&hSLC)))
	{	// Read all license key info using SLGetLicensingStatusInformation()
		UINT nStatusCount = 0;
		SL_LICENSING_STATUS *pLicensingStatus = nullptr;
		if(SUCCEEDED(SLGetLicensingStatusInformation(hSLC, &guidAppId, nullptr, nullptr, &nStatusCount, &pLicensingStatus)))
		{
			for(UINT nStatus = 0; nStatus < nStatusCount; ++nStatus)
			{
				if(pLicensingStatus[nStatus].eStatus == SL_LICENSING_STATUS_LICENSED)
					std::wcout << L"Status: licensed" << std::endl;
				else if(pLicensingStatus[nStatus].eStatus == SL_LICENSING_STATUS_IN_GRACE_PERIOD)
				{
					std::wcout << L"Status: grace period" << std::endl;
					std::wcout << L"Total grace days: " << pLicensingStatus[nStatus].dwTotalGraceDays << std::endl;
					std::wcout << L"Grace time in minutes: " << pLicensingStatus[nStatus].dwGraceTime << std::endl;
				}
				else if(pLicensingStatus[nStatus].eStatus == SL_LICENSING_STATUS_NOTIFICATION)
					std::wcout << L"Status: notification" << std::endl;

				if(pLicensingStatus[nStatus].eStatus == SL_LICENSING_STATUS_LICENSED || pLicensingStatus[nStatus].eStatus == SL_LICENSING_STATUS_IN_GRACE_PERIOD || pLicensingStatus[nStatus].eStatus == SL_LICENSING_STATUS_NOTIFICATION)
				{
					UINT nKeyCount = 0;
					SLID *pKeys = nullptr;
					if(SUCCEEDED(SLGetInstalledProductKeyIds(hSLC, &pLicensingStatus[nStatus].SkuId, &nKeyCount, &pKeys)))
					{
						for(UINT nKey = 0; nKey < nKeyCount; ++nKey)
						{
							SLDATATYPE type;
							UINT nSize = 0;
							PBYTE pBuffer = nullptr;

							if(SUCCEEDED(SLGetPKeyInformation(hSLC, &pKeys[nKey], SL_INFO_KEY_CHANNEL, &type, &nSize, &pBuffer)))
							{
								if(type == SL_DATA_SZ)
								{
									str.assign((PCWSTR)pBuffer, nSize / sizeof(wchar_t));
									std::wcout << L"Channel: " << str << std::endl;
								}

								LocalFree(pBuffer);
							}

							if(SUCCEEDED(SLGetPKeyInformation(hSLC, &pKeys[nKey], SL_INFO_KEY_DIGITAL_PID, &type, &nSize, &pBuffer)))
							{
								if(type == SL_DATA_SZ)
								{
									str.assign((PCWSTR)pBuffer, nSize / sizeof(wchar_t));
									std::wcout << L"DigitalPID: " << str << std::endl;
								}

								LocalFree(pBuffer);
							}

							if(SUCCEEDED(SLGetPKeyInformation(hSLC, &pKeys[nKey], SL_INFO_KEY_DIGITAL_PID2, &type, &nSize, &pBuffer)))
							{
								if(type == SL_DATA_SZ)
								{
									str.assign((PCWSTR)pBuffer, nSize / sizeof(wchar_t));
									std::wcout << L"DigitalPID2: " << str << std::endl;
								}

								LocalFree(pBuffer);
							}

							if(SUCCEEDED(SLGetPKeyInformation(hSLC, &pKeys[nKey], SL_INFO_KEY_PARTIAL_PRODUCT_KEY, &type, &nSize, &pBuffer)))
							{
								if(type == SL_DATA_SZ)
								{
									str.assign((PCWSTR)pBuffer, nSize / sizeof(wchar_t));
									std::wcout << L"Partial key: #####-#####-#####-#####-" << str << std::endl;
								}

								LocalFree(pBuffer);
							}
						}

						LocalFree(pKeys);
					}
				}
			}

			LocalFree(pLicensingStatus);
		}

		SLClose(hSLC);
	}

	return NO_ERROR;
}

//#################################################################################################
void Convert8ToW(const std::string &str8, std::wstring &strW)
{
	strW.clear();

	if(!str8.empty())
	{	// Alloc and copy
		int nLenW = MultiByteToWideChar(CP_UTF8, 0, str8.c_str(), (int)str8.size(), nullptr, 0);
		auto pBuffer = std::make_unique<wchar_t[]>(nLenW + 1);

		if(MultiByteToWideChar(CP_UTF8, 0, str8.c_str(), (int)str8.size(), pBuffer.get(), nLenW + 1))
		{
			pBuffer.get()[nLenW] = L'\0';
			strW = pBuffer.get();
		}
	}
}

//#################################################################################################
std::wstring GetBiosLicenseKey(void)
{
	std::wstring strLicenseKey;

	struct AcpiTableHeader final
	{
		char chSignature[4];
		uint32_t nSize;
		uint8_t nRevision;
		uint8_t nChecksum;
		char chOemId[6];
		char chOemTableId[8];
		uint32_t nOemRevision;
		char chCreatorId[4];
		uint32_t nCreatorRevision;
		uint32_t nVersion;
		uint32_t nReserved;
		uint32_t nDataType;
		uint32_t nDataReserved;
		uint32_t nDataSize;
	};
	using PAcpiTableHeader = AcpiTableHeader*;

	// The Windows license key (if present) is encoded in the Microsoft Data Management firmware table
	DWORD dwSize = GetSystemFirmwareTable('ACPI', 'MDSM', nullptr, 0);
	if(dwSize > 8)
	{
		auto pBuf = std::make_unique<BYTE[]>(dwSize);
		if(GetSystemFirmwareTable('ACPI', 'MDSM', pBuf.get(), dwSize) == dwSize)
		{
			PAcpiTableHeader pHeader = (PAcpiTableHeader)pBuf.get();

			// Windows license keys are 29 characters long, 30 if null-terminated
			if(pHeader->nDataSize == 29 || pHeader->nDataSize == 30)
			{
				std::string strLicenseKey8((PCSTR)pBuf.get() + sizeof(AcpiTableHeader), pHeader->nDataSize);
				Convert8ToW(strLicenseKey8, strLicenseKey);
			}
		}
	}

	return strLicenseKey;
}

//#################################################################################################
std::wstring GetWindowsActivationKey(void)
{
	std::wstring strKey;

	static constexpr auto KEY_OFFSET = 52;

	// The license key used to activate Windows may be stored in 'DigitalProductId'
	// However, if the machine was activated using a Multiple Activation Key (MAK) then the key is discarded and only gargabe is saved in the registry
	HKEY hKey;
	if(RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwDataType;
		DWORD dwDataSize;
		RegQueryValueExW(hKey, L"DigitalProductId", nullptr, &dwDataType, nullptr, &dwDataSize);
		if(dwDataType == REG_BINARY && dwDataSize > 66)
		{
			auto pBuffer = std::make_unique<BYTE[]>(dwDataSize);
			if(RegQueryValueExW(hKey, L"DigitalProductId", nullptr, &dwDataType, pBuffer.get(), &dwDataSize) == ERROR_SUCCESS)
			{
				auto pBuf = pBuffer.get();
				int nWin8 = ((pBuf[66] / 6) & 1);
				int nLast;

				pBuf[66] = (pBuf[66] & 0xF7) | ((nWin8 & 2) * 4);

				PCWSTR szMap = L"BCDFGHJKMPQRTVWXY2346789";
				for(int i = 0; i < 25; ++i)
				{
					int nCurrent = 0;
					for(int j = 14; j >= 0; --j)
					{
						nCurrent = nCurrent * 256;
						nCurrent = pBuffer[j + KEY_OFFSET] + nCurrent;
						pBuffer[j + KEY_OFFSET] = (BYTE)(nCurrent / 24);
						nCurrent = nCurrent % 24;
					}

					strKey.insert(0, 1, szMap[nCurrent]);
					nLast = nCurrent;
				}

				if(nWin8)
					strKey.insert(nLast + 1, 1, L'N');

				// Remove the first character then add the hyphens
				strKey.erase(0, 1);
				strKey.insert(5, 1, L'-');
				strKey.insert(11, 1, L'-');
				strKey.insert(17, 1, L'-');
				strKey.insert(23, 1, L'-');
			}
		}

		RegCloseKey(hKey);
	}

	return strKey;
}
