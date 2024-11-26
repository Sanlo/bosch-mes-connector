#ifndef POLYWORKS_H
#define POLYWORKS_H
#include <atlcomcli.h>
#include <atlsafe.h>
#include <string>

class IIMWorkspaceManager2;
class IIMInspect;
class IIMCommandCenter;

class PolyWorks
{
public:
    enum ModuleType {
        MODULE_WORKSPACE,
        MODULE_INSPECTOR,
    };

    explicit PolyWorks();
    ~PolyWorks();
    bool scriptExecute(ModuleType module, const OLECHAR *filePath, const OLECHAR *arg);

    inline int returnCode() const { return m_returnCode; }
    inline const std::wstring returnMessage() const { return m_returnMsg; }

private:
    void startWorkspaceManager();

    IIMWorkspaceManager2 *m_pIMWorkspaceManager2;
    IIMCommandCenter *m_pIMInspectCommandCenter;
    IIMCommandCenter *m_pWSMCommandCenter;

    int m_returnCode;
    std::wstring m_returnMsg;
};

#endif // POLYWORKS_H
