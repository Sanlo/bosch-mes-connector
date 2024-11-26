
#include <PolyWorksSDK/COM/IIMInspect.h>
#include <PolyWorksSDK/COM/IIMInspect_i.c>
#include <PolyWorksSDK/COM/IIMWorkspaceManager2.h>
#include <PolyWorksSDK/COM/IIMWorkspaceManager2_i.c>
#include <PolyWorksSDK/COM/IMInspect_i.c>
#include <PolyWorksSDK/COM/IMWorkspaceManager_i.c>

#include "polyworks.h"

PolyWorks::PolyWorks()
    : m_pIMWorkspaceManager2(nullptr)
    , m_pIMInspectCommandCenter(nullptr)
    , m_pWSMCommandCenter(nullptr)
{
    startWorkspaceManager();
}

PolyWorks::~PolyWorks()
{
    if (m_pIMInspectCommandCenter) {
        long lReturnValue;
        m_pIMInspectCommandCenter->CommandExecute(L"FILE EXIT", &lReturnValue);
        m_pWSMCommandCenter->CommandExecute(L"FILE EXIT", &lReturnValue);
        m_pIMInspectCommandCenter->Release();
        m_pWSMCommandCenter->Release();
        m_pIMWorkspaceManager2->Release();
        CoUninitialize();
    }
}

bool PolyWorks::scriptExecute(ModuleType module, const OLECHAR *filePath, const OLECHAR *arg)
{
    long retVal;
    long plsSuccess;
    if (ModuleType::MODULE_WORKSPACE == module && m_pWSMCommandCenter) {
        // Run Workspace Manager Macro script file
        if (SUCCEEDED(m_pWSMCommandCenter->ScriptExecuteFromFile(filePath, arg, &retVal)) && (retVal == 1)) {
            long valIndex;
            m_pWSMCommandCenter->ScriptVariableGetNbValues(L"returnCode_", &valIndex);
            if (SUCCEEDED(
                    m_pWSMCommandCenter->ScriptVariableGetValueAsInt(L"returnCode_", valIndex, &m_returnCode, &retVal))
                && SUCCEEDED(m_pWSMCommandCenter->ReturnValueIsSuccess(retVal, &plsSuccess)) && plsSuccess) {
            }

            BSTR str = NULL;
            m_pWSMCommandCenter->ScriptVariableGetNbValues(L"returnMsg_", &valIndex);
            if (SUCCEEDED(m_pWSMCommandCenter->ScriptVariableGetValueAsString(L"returnMsg_", valIndex, &str, &retVal))
                && SUCCEEDED(m_pWSMCommandCenter->ReturnValueIsSuccess(retVal, &plsSuccess)) && plsSuccess) {
                m_returnMsg = str;
            }
            ::SysFreeString(str);
            return true;
        }
    }

    return true;
}

void PolyWorks::startWorkspaceManager()
{
    HRESULT hr = 0;
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (!SUCCEEDED(hr)) {
        CoUninitialize();
        return;
    }

    hr = CoCreateInstance(CLSID_IMWorkspaceManager,
                          NULL,
                          CLSCTX_SERVER,
                          IID_IIMWorkspaceManager2,
                          (void **) &m_pIMWorkspaceManager2);
    if (!SUCCEEDED(hr) || m_pIMWorkspaceManager2 == nullptr) {
        CoUninitialize();
        return;
    }

    if (!SUCCEEDED(m_pIMWorkspaceManager2->CommandCenterCreate(&m_pWSMCommandCenter))
        || m_pWSMCommandCenter == nullptr) {
        CoUninitialize();
        return;
    }
}

