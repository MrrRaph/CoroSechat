package com.github.MrrRaph.corosechat.server.utils;

import com.github.MrrRaph.corosechat.server.communications.ClientCommand;

import java.io.PrintWriter;

public final class Writifier {
    public static void systemWriter(PrintWriter pw, String message) {
        pw.println("<SYSTEM> " + message);
    }

    public static void systemWriter(PrintWriter pw, ClientCommand command) {
        pw.println("<SYSTEM> " + command);
    }

    public static void messageWriter(PrintWriter pw, String username, String message, Boolean isAdmin) {
        if (!isAdmin) pw.println("[" + username + "] " + message);
        else pw.println(username + " " + message);
    }
}
