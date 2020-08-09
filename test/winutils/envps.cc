#include <bela/simulator.hpp>

int LinkToApp(wchar_t *env) {
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  SecureZeroMemory(&si, sizeof(si));
  SecureZeroMemory(&pi, sizeof(pi));
  si.cb = sizeof(si);
  wchar_t cmd[256] = L"pwsh";
  if (!CreateProcessW(nullptr, cmd, nullptr, nullptr, FALSE, CREATE_UNICODE_ENVIRONMENT, env,
                      nullptr, &si, &pi)) {
    return -1;
  }
  CloseHandle(pi.hThread);
  SetConsoleCtrlHandler(nullptr, TRUE);
  WaitForSingleObject(pi.hProcess, INFINITE);
  SetConsoleCtrlHandler(nullptr, FALSE);
  DWORD exitCode;
  GetExitCodeProcess(pi.hProcess, &exitCode);
  CloseHandle(pi.hProcess);
  return exitCode;
}

int wmain() {
  bela::env::Simulator simulator;
  simulator.InitializeCleanupEnv();
  simulator.SetEnv(L"GOPROXY", L"https://goproxy.io/");
  // simulator.SetEnv(L"Path", L"C:/Dev");
  auto envs = simulator.MakeEnv();
  return LinkToApp(envs.data());
}