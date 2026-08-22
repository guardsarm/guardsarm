/* GuardsArm default YARA detection pack — real file-content signatures. */

rule GuardsArm_PHP_Webshell {
    meta:
        description = "PHP webshell indicators (eval/base64/exec on request input)"
        severity = "critical"
        mitre = "T1505.003"
    strings:
        $eval = "eval(" $b64 = "base64_decode(" $sys = "system(" $shell = "shell_exec("
        $pass = "passthru(" $post = "$_POST[" $get = "$_GET[" $req = "$_REQUEST["
    condition:
        filesize < 300KB and #eval + #b64 + #sys + #shell + #pass > 0 and any of ($post,$get,$req) and 3 of them
}

rule GuardsArm_Download_Execute {
    meta:
        description = "Download-and-execute / reverse-shell shell script"
        severity = "high"
        mitre = "T1059/T1105"
    strings:
        $curl = "curl " $wget = "wget " $psh = "|sh" $pbash = "|bash"
        $chmod = "chmod +x" $devtcp = "/dev/tcp/" $nc = "nc -e" $b64d = "base64 -d"
    condition:
        filesize < 200KB and 2 of them
}

rule GuardsArm_Ransom_Note {
    meta:
        description = "Ransomware ransom-note indicators"
        severity = "critical"
        mitre = "T1486"
    strings:
        $enc = "your files have been encrypted" nocase
        $enc2 = "files are encrypted" nocase
        $btc = "bitcoin" nocase $dec = "decrypt" nocase $onion = ".onion" nocase
        $ransom = "ransom" nocase
    condition:
        filesize < 1MB and 2 of them
}

rule GuardsArm_ELF_Backdoor {
    meta:
        description = "ELF binary with socket + exec + shell (possible backdoor)"
        severity = "high"
        mitre = "T1059.004"
    strings:
        $sh = "/bin/sh" $socket = "socket" $connect = "connect" $execve = "execve"
        $bindsh = "bindshell" nocase $reverse = "reverse" nocase
    condition:
        uint32(0) == 0x464c457f and 4 of them
}

rule GuardsArm_Suspicious_Python {
    meta:
        description = "Python one-liner reverse shell / exec"
        severity = "high"
        mitre = "T1059.006"
    strings:
        $sock = "socket.socket" $conn = ".connect(" $sub = "subprocess" $pty = "pty.spawn"
        $os = "os.system(" $b64 = "base64.b64decode" $exec = "exec("
    condition:
        filesize < 100KB and 3 of them
}

/* =====================================================================
   MALWARE & RANSOMWARE FAMILY SIGNATURES
   Real content signatures (public tradecraft) for well-known families.
   Scanned on-agent by guardsarm-yara-scan (libyara) over files + process
   images; matches route to the Malware Center via edr_malware.
   ===================================================================== */

rule GuardsArm_EICAR_Test {
    meta:
        description = "EICAR standard anti-malware test file"
        severity = "low"
        mitre = "T1204"
    strings:
        $eicar = "X5O!P%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*"
    condition:
        $eicar
}

rule GuardsArm_Mimikatz {
    meta:
        description = "Mimikatz credential-dumping toolkit"
        severity = "critical"
        mitre = "T1003.001"
    strings:
        $a = "sekurlsa" nocase
        $b = "gentilkiwi" nocase
        $c = "mimikatz" nocase
        $d = "logonpasswords" nocase
        $e = "kerberos::" nocase
        $f = "privilege::debug" nocase
        $g = "sekurlsa::logonpasswords" nocase
    condition:
        2 of them
}

rule GuardsArm_CobaltStrike_Beacon {
    meta:
        description = "Cobalt Strike Beacon implant indicators"
        severity = "critical"
        mitre = "T1055"
    strings:
        $p1 = "\\\\.\\pipe\\msagent" nocase
        $p2 = "\\\\.\\pipe\\MSSE-" nocase
        $p3 = "\\\\.\\pipe\\status_" nocase
        $p4 = "\\\\.\\pipe\\postex_" nocase
        $s1 = "beacon.dll" nocase
        $s2 = "ReflectiveLoader"
        $s3 = "%s as %s\\%s: %d"
        $s4 = "Started service %s on %s"
        $s5 = "could not spawn %s"
    condition:
        2 of them
}

rule GuardsArm_Meterpreter {
    meta:
        description = "Metasploit Meterpreter payload"
        severity = "critical"
        mitre = "T1059"
    strings:
        $a = "metsrv"
        $b = "meterpreter" nocase
        $c = "stdapi_"
        $d = "core_channel_open"
        $e = "priv_passwd_get_sam_hashes"
    condition:
        2 of them
}

rule GuardsArm_WannaCry {
    meta:
        description = "WannaCry ransomware (WannaCrypt0r)"
        severity = "critical"
        mitre = "T1486"
    strings:
        $a = "WNcry@2ol7"
        $b = "@WanaDecryptor@"
        $c = "taskdl.exe"
        $d = "taskse.exe"
        $e = "mssecsvc.exe"
        $f = "tasksche.exe"
        $g = "WanaCrypt0r" nocase
    condition:
        2 of them
}

rule GuardsArm_Ransom_LockBit {
    meta:
        description = "LockBit ransomware indicators"
        severity = "critical"
        mitre = "T1486"
    strings:
        $a = "Restore-My-Files.txt" nocase
        $b = "LockBit"
        $c = "restore-my-files" nocase
        $d = "All your important files are encrypted" nocase
        $e = ".lockbit" nocase
    condition:
        2 of them
}

rule GuardsArm_Ransom_Conti {
    meta:
        description = "Conti ransomware indicators"
        severity = "critical"
        mitre = "T1486"
    strings:
        $a = "CONTI_LOG.txt" nocase
        $b = "R3ADM3.txt" nocase
        $c = "conti" nocase
        $d = "your system was hacked" nocase
        $e = "The network is LOCKED" nocase
    condition:
        2 of them
}

rule GuardsArm_Ransomware_Note_Families {
    meta:
        description = "Family-specific ransom-note artifacts (Ryuk/Medusa/Akira/Cl0p/BlackCat/etc.)"
        severity = "critical"
        mitre = "T1486"
    strings:
        $f1 = "RyukReadme" nocase
        $f2 = "readme_for_decrypt" nocase
        $f3 = "how_to_back_files" nocase
        $f4 = "READ_ME_MEDUSA" nocase
        $f5 = "akira_readme" nocase
        $f6 = "ClopReadme" nocase
        $f7 = "RECOVER-FILES" nocase
        $f8 = "Restore-My-Files" nocase
        $f9 = "HOW_TO_DECRYPT" nocase
        $f10 = "CriticalBreachDetected" nocase
        $g1 = "tor browser" nocase
        $g2 = "bitcoin" nocase
        $g3 = "your files have been encrypted" nocase
        $g4 = ".onion" nocase
    condition:
        any of ($f*) or (3 of ($g*) and filesize < 500KB)
}

rule GuardsArm_China_Chopper_Webshell {
    meta:
        description = "China Chopper compact webshell"
        severity = "critical"
        mitre = "T1505.003"
    strings:
        $asp = "<%@ Page Language=\"Jscript\"%>" nocase
        $eval1 = "eval(Request" nocase
        $eval2 = "eval(request.item" nocase
        $php = "@eval($_POST" nocase
        $asp2 = "execute(request" nocase
    condition:
        filesize < 50KB and any of them
}

rule GuardsArm_Generic_Webshell {
    meta:
        description = "Known PHP/ASP webshell families (WSO/b374k/c99/r57)"
        severity = "critical"
        mitre = "T1505.003"
    strings:
        $wso = "wso shell" nocase
        $b374k = "b374k" nocase
        $c99 = "c99shell" nocase
        $r57 = "r57shell" nocase
        $fm = "FilesMan" nocase
        $up = "uploadFile" nocase
        $safe = "safe_mode" nocase
        $pd = "passthru(" nocase
    condition:
        filesize < 500KB and 2 of them
}

rule GuardsArm_CryptoMiner {
    meta:
        description = "Cryptocurrency miner (XMRig/cpuminer)"
        severity = "high"
        mitre = "T1496"
    strings:
        $a = "xmrig" nocase
        $b = "stratum+tcp://" nocase
        $c = "stratum+ssl://" nocase
        $d = "cryptonight" nocase
        $e = "randomx" nocase
        $f = "--donate-level"
        $g = "minerd"
    condition:
        2 of them
}

rule GuardsArm_Mirai_IoT_Botnet {
    meta:
        description = "Mirai/Gafgyt IoT botnet ELF"
        severity = "critical"
        mitre = "T1498"
    strings:
        $a = "/bin/busybox"
        $b = "MIRAI"
        $c = "TSource Engine Query"
        $d = "/dev/watchdog"
        $e = "gayfgt" nocase
        $f = "LCOGKMBOEA"
        $g = "hackers" nocase
    condition:
        uint32(0) == 0x464c457f and 2 of them
}

rule GuardsArm_DotNet_RAT {
    meta:
        description = ".NET commodity RAT (AsyncRAT/Quasar/njRAT)"
        severity = "critical"
        mitre = "T1219"
    strings:
        $async = "AsyncRAT" nocase
        $quasar = "Quasar" nocase
        $nj = "njrat" nocase
        $nj2 = "njq8" nocase
        $kl = "Keylogger" nocase
        $kl2 = "[ENTER]"
        $cfg = "Pastebin" nocase
        $stub = "Client.exe"
    condition:
        uint16(0) == 0x5a4d and 2 of them
}

rule GuardsArm_Stealer_AgentTesla {
    meta:
        description = "Agent Tesla / commodity infostealer"
        severity = "critical"
        mitre = "T1555"
    strings:
        $a = "AgentTesla" nocase
        $b = "MailPassView" nocase
        $c = "WebBrowserPassView" nocase
        $d = "Vault Schema Guid" nocase
        $e = "logins" nocase
        $f = "smtp" nocase
        $g = "Postbox" nocase
    condition:
        uint16(0) == 0x5a4d and 3 of them
}

rule GuardsArm_UPX_Packed {
    meta:
        description = "UPX-packed executable (common malware obfuscation)"
        severity = "medium"
        mitre = "T1027.002"
    strings:
        $u0 = "UPX0"
        $u1 = "UPX1"
        $u2 = "UPX!"
    condition:
        (uint16(0) == 0x5a4d or uint32(0) == 0x464c457f) and 2 of them
}
