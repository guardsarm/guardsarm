/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <process.h>
#include "string_op.h"
#include "os_win32ui.h"
#include "../os_win.h"
#include "dll_load_notify.h"

guardsarm_config config_inst;
HWND hStatus;

/* Dialog -- About GUARDSARM */
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message,
       WPARAM wParam,
       __attribute__((unused))LPARAM lParam)
{
    switch (Message) {
        case WM_CREATE:
        case WM_INITDIALOG:

            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case UI_ID_CLOSE:
                    EndDialog(hwnd, IDOK);
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwnd, IDOK);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

/* Main Dialog */
BOOL CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam,
        __attribute__((unused))LPARAM lParam)
{
    int ret_code = 0;


    switch (Message) {
        case WM_INITDIALOG: {
            int statwidths[] = {130, -1};
            HMENU hMenu, hSubMenu;

            UINT menuflags = MF_STRING;

            if (config_inst.admin_access == 0) {
                menuflags = MF_STRING | MF_GRAYED;
            }

            hMenu = CreateMenu();

            /* Creating management menu */
            hSubMenu = CreatePopupMenu();
            AppendMenu(hSubMenu, menuflags, UI_MENU_MANAGE_START, "&Start");
            AppendMenu(hSubMenu, menuflags, UI_MENU_MANAGE_STOP, "&Stop");
            AppendMenu(hSubMenu, menuflags, UI_MENU_MANAGE_RESTART, "&Restart");
            AppendMenu(hSubMenu, MF_SEPARATOR, UI_MENU_NONE, "");
            AppendMenu(hSubMenu, menuflags, UI_MENU_MANAGE_STATUS, "&Status");
            AppendMenu(hSubMenu, MF_SEPARATOR, UI_MENU_NONE, "");
            AppendMenu(hSubMenu, MF_STRING, UI_MENU_MANAGE_EXIT, "&Exit");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Manage");

            /* Create view menu */
            hSubMenu = CreatePopupMenu();
            AppendMenu(hSubMenu, MF_STRING, UI_MENU_VIEW_LOGS, "&View Logs");
            AppendMenu(hSubMenu, MF_STRING, UI_MENU_VIEW_CONFIG, "V&iew Config");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&View");

            hSubMenu = CreatePopupMenu();
            AppendMenu(hSubMenu, MF_STRING, UI_MENU_HELP_ABOUT, "A&bout");
            AppendMenu(hSubMenu, MF_STRING, UI_MENU_HELP_HELP, "Help");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Help");


            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            SetMenu(hwnd, hMenu);


            hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
                                     WS_CHILD | WS_VISIBLE,
                                     0, 0, 0, 0,
                                     hwnd, (HMENU)IDC_MAIN_STATUS,
                                     GetModuleHandle(NULL), NULL);

            SendMessage(hStatus, SB_SETPARTS,
                        sizeof(statwidths) / sizeof(int),
                        (LPARAM)statwidths);
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"https://guardsarmsiem.com");


            /* Initializing config */
            config_read(hwnd);
            gen_server_info(hwnd);


            /* Setting the icons */
            SendMessage(hwnd, WM_SETICON, ICON_SMALL,
                        (LPARAM)LoadIcon(GetModuleHandle(NULL),
                                         MAKEINTRESOURCE(IDI_OSSECICON)));
            SendMessage(hwnd, WM_SETICON, ICON_BIG,
                        (LPARAM)LoadIcon(GetModuleHandle(NULL),
                                         MAKEINTRESOURCE(IDI_OSSECICON)));

            if (config_inst.admin_access == 0) {
                MessageBox(hwnd, "Admin access required. Some features may not work properly. \n\n"
                           "**If on Vista (or Server 2008), choose the \"Run as administrator\" option.",
                           "Admin Access Required", MB_OK);
                break;
            }

        }
        break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                /* In case of SAVE */
                case IDC_ADD: {
                    int chd = 0;
                    int len;

                    if (config_inst.admin_access == 0) {
                        MessageBox(hwnd, "Unable to edit configuration. "
                                   "Admin access required.",
                                   "Error Saving.", MB_OK);
                        break;
                    }

                    /* Get server IP */
                    len = GetWindowTextLength(GetDlgItem(hwnd, UI_SERVER_TEXT));
                    if (len > 0) {
                        char *buf;

                        /* Allocate buffer */
                        buf = (char *)GlobalAlloc(GPTR, len + 1);
                        if (!buf) {
                            exit(-1);
                        }

                        GetDlgItemText(hwnd, UI_SERVER_TEXT, buf, len + 1);

                        /* If auth key changed, set it */
                        if (strcmp(buf, config_inst.server) != 0) {
                            if (set_ossec_server(buf, hwnd)) {
                                chd = 1;
                            }
                        }
                        GlobalFree(buf);
                    }

                    /* Get auth key */
                    len = GetWindowTextLength(GetDlgItem(hwnd, UI_SERVER_AUTH));
                    if (len > 0) {
                        char *buf;

                        /* Allocate buffer */
                        buf = (char *)GlobalAlloc(GPTR, len + 1);
                        if (!buf) {
                            exit(-1);
                        }

                        GetDlgItemText(hwnd, UI_SERVER_AUTH, buf, len + 1);

                        /* If auth key changed, set it */
                        if (strcmp(buf, config_inst.key) != 0) {
                            int ret;
                            char *tmp_str;
                            char *decd_buf = NULL;
                            char *decd_to_write = NULL;
                            char *id = NULL;
                            char *name = NULL;
                            char *ip = NULL;

                            /* Get new fields */
                            decd_buf = decode_base64(buf);
                            if (decd_buf) {
                                decd_to_write = strdup(decd_buf);

                                /* Get ID, name and IP */
                                id = decd_buf;
                                name = strchr(id, ' ');
                                if (name) {
                                    *name = '\0';
                                    name++;

                                    ip = strchr(name, ' ');
                                    if (ip) {
                                        *ip = '\0';
                                        ip++;

                                        tmp_str = strchr(ip, ' ');
                                        if (tmp_str) {
                                            *tmp_str = '\0';
                                        }
                                    }
                                }
                            }

                            /* If IP isn't set, it is because we have an invalid
                             * auth key.
                             */
                            if (!ip) {
                                MessageBox(hwnd, "Unable to import "
                                           "authentication key because it was invalid.",
                                           "Error -- Failure Saving Auth Key", MB_OK);
                            } else {
                                char mbox_msg[1024 + 1];
                                mbox_msg[1024] = '\0';

                                snprintf(mbox_msg, 1024, "Adding key for:\r\n\r\n"
                                         "Agent ID: %s\r\n"
                                         "Agent Name: %s\r\n"
                                         "IP Address: %s\r\n",
                                         id, name, ip);

                                ret = MessageBox(hwnd, mbox_msg,
                                                 "Confirm Importing Key", MB_OKCANCEL);
                                if (ret == IDOK) {
                                    if (set_ossec_key(decd_to_write, hwnd)) {
                                        chd += 2;
                                    }
                                }
                            }

                            /* Free used memory */
                            if (decd_buf) {
                                free(decd_to_write);
                                free(decd_buf);
                            }
                        }
                        GlobalFree(buf);
                    } /* Finished adding AUTH KEY */

                    /* Re-print messages */
                    if (chd) {
                        config_read(hwnd);

                        /* Set status to restart */
                        if (strcmp(config_inst.status, ST_RUNNING) == 0) {
                            config_inst.status = ST_RUNNING_RESTART;
                        }

                        gen_server_info(hwnd);

                        if (chd == 1) {
                            SendMessage(hStatus, SB_SETTEXT, 0,
                                        (LPARAM)"Manager IP saved");
                        } else if (chd == 2) {
                            SendMessage(hStatus, SB_SETTEXT, 0,
                                        (LPARAM)"Auth key imported");

                        } else {
                            SendMessage(hStatus, SB_SETTEXT, 0,
                                        (LPARAM)"Auth key and IP saved");

                        }
                    }
                }
                break;

                /* In case of ENROLL -- auto-register with the manager */
                case IDC_ENROLL: {
                    int len_srv, len_pass;
                    char *srv = NULL, *pass = NULL;

                    if (config_inst.admin_access == 0) {
                        MessageBox(hwnd, "Unable to enroll. Admin access required.\r\n"
                                   "Run the GuardSarm Agent as administrator.",
                                   "Enrollment", MB_OK | MB_ICONWARNING);
                        break;
                    }

                    len_srv  = GetWindowTextLength(GetDlgItem(hwnd, UI_SERVER_TEXT));
                    len_pass = GetWindowTextLength(GetDlgItem(hwnd, UI_ENROLL_PASS));
                    if (len_srv <= 0 || len_pass <= 0) {
                        MessageBox(hwnd, "Enter both the manager address and the enrollment "
                                   "password before clicking Enroll.",
                                   "Enrollment", MB_OK | MB_ICONWARNING);
                        break;
                    }

                    srv  = (char *)GlobalAlloc(GPTR, len_srv + 1);
                    pass = (char *)GlobalAlloc(GPTR, len_pass + 1);
                    if (!srv || !pass) {
                        exit(-1);
                    }
                    GetDlgItemText(hwnd, UI_SERVER_TEXT, srv, len_srv + 1);
                    GetDlgItemText(hwnd, UI_ENROLL_PASS, pass, len_pass + 1);

                    {
                        char msg[512];
                        snprintf(msg, sizeof(msg),
                                 "Enroll this agent with manager:\r\n\r\n    %s\r\n\r\n"
                                 "The agent will register automatically and restart. Continue?",
                                 srv);
                        if (MessageBox(hwnd, msg, "Confirm Enrollment",
                                       MB_OKCANCEL | MB_ICONQUESTION) != IDOK) {
                            GlobalFree(srv);
                            GlobalFree(pass);
                            break;
                        }
                    }

                    /* Apply the manager address + one-time enrollment password */
                    if (!set_ossec_server(srv, hwnd) || !set_enroll_pass(pass, hwnd)) {
                        GlobalFree(srv);
                        GlobalFree(pass);
                        break;
                    }

                    /* Never leave the one-time secret on screen */
                    SetDlgItemText(hwnd, UI_ENROLL_PASS, "");

                    /* Restart so the agent registers with the manager */
                    os_stop_service();
                    if (os_start_service() == 0) {
                        MessageBox(hwnd, "Saved the manager address and enrollment password, but "
                                   "could not start the agent. Use Manage > Start.",
                                   "Enrollment", MB_OK | MB_ICONWARNING);
                    } else {
                        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Enrolling with manager...");
                        MessageBox(hwnd, "Enrollment started. The agent is registering with the "
                                   "manager and will connect shortly.\r\n\r\n"
                                   "Click Refresh in a few seconds to see the assigned Agent ID.",
                                   "Enrollment", MB_OK | MB_ICONINFORMATION);
                    }

                    config_read(hwnd);
                    gen_server_info(hwnd);

                    GlobalFree(srv);
                    GlobalFree(pass);
                }
                break;

                case UI_MENU_MANAGE_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;

                case UI_MENU_VIEW_LOGS:
                    _spawnlp( _P_NOWAIT, "notepad", "notepad " OSSECLOGS, NULL );
                    break;
                case UI_MENU_VIEW_CONFIG:
                    _spawnlp( _P_NOWAIT, "notepad", "notepad " CONFIG, NULL );
                    break;
                case UI_MENU_HELP_HELP:
                    _spawnlp( _P_NOWAIT, "notepad", "notepad " HELPTXT, NULL );
                    break;
                case UI_MENU_HELP_ABOUT: {
                    DialogBox(GetModuleHandle(NULL),
                              MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
                }
                break;
                case IDC_CANCEL:
                    config_read(hwnd);
                    gen_server_info(hwnd);
                    break;

                case UI_MENU_MANAGE_START:

                    /* Start GUARDSARM  -- must have a valid config before */
                    if (strcmp(config_inst.server, FL_NOSERVER) != 0) {
                        ret_code = os_start_service();
                    } else {
                        ret_code = 0;
                    }

                    if (ret_code == 0) {
                        MessageBox(hwnd, "Unable to start agent (check config)",
                                   "Error -- Unable to Start Agent", MB_OK);
                    } else if (ret_code == 1) {
                        config_read(hwnd);
                        gen_server_info(hwnd);

                        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Started");

                        MessageBox(hwnd, "Agent started",
                                   "Agent Started", MB_OK);
                    } else {
                        MessageBox(hwnd, "Agent already running (try restart)",
                                   "Agent Running", MB_OK);
                    }
                    break;
                case UI_MENU_MANAGE_STOP:

                    /* Stop GUARDSARM */
                    ret_code = os_stop_service();
                    if (ret_code == 1) {
                        config_read(hwnd);
                        gen_server_info(hwnd);

                        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) "Stopped");
                        MessageBox(hwnd, "Agent stopped", "Agent Stopped", MB_OK);
                    }
                    else if (ret_code == -1) {
                        MessageBox(hwnd, "Agent already stopped", "Agent Stopped", MB_OK);
                    }
                    else {
                        MessageBox(hwnd, "Unable to stop agent", "Error -- Unable to Stop Agent", MB_OK);
                    }
                    break;
                case UI_MENU_MANAGE_STATUS:
                    if (CheckServiceRunning()) {
                        MessageBox(hwnd, "Agent running",
                                   "Agent Running", MB_OK);

                    } else {
                        MessageBox(hwnd, "Agent stopped",
                                   "Agent Stopped", MB_OK);
                    }
                    break;
                case UI_MENU_MANAGE_RESTART:

                    if (strcmp(config_inst.server, FL_NOSERVER) == 0) {
                        MessageBox(hwnd, "Unable to restart agent (check config)",
                                   "Error -- Unable to Restart Agent", MB_OK);
                        break;

                    }

                    os_stop_service();

                    /* Start GUARDSARM */
                    ret_code = os_start_service();
                    if (ret_code == 0) {
                        MessageBox(hwnd, "Unable to restart agent (check config)",
                                   "Error -- Unable to Restart Agent", MB_OK);
                    } else {
                        config_read(hwnd);
                        gen_server_info(hwnd);

                        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Restarted");
                        MessageBox(hwnd, "Agent restarted",
                                   "Agent Restarted", MB_OK);
                    }
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwnd, 0);
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, __attribute__((unused))HINSTANCE hPrevInstance,
        __attribute__((unused))LPSTR lpCmdLine, __attribute__((unused))int nCmdShow)
{
    // This must be always the first instruction
    enable_dll_verification();

    WSADATA wsaData;

    /* Start Winsock -- for name resolution */
    WSAStartup(MAKEWORD(2, 0), &wsaData);

    /* Initialize config */
    init_config();

    /* Initialize controls */
    InitCommonControls();

    /* Create main dialogbox */
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProc);

    return (0);
}
