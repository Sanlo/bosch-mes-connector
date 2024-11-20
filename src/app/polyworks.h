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

private:
    void startWorkspaceManager();
    void initInspector(const OLECHAR *prjName);

    IIMWorkspaceManager2 *m_pIMWorkspaceManager2;
    IIMCommandCenter *m_pIMInspectCommandCenter;
    IIMCommandCenter *m_pWSMCommandCenter;

    std::wstring m_projectName;
};

#endif // POLYWORKS_H
