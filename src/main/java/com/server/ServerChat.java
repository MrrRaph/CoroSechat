package main.java.com.server;

import main.java.com.logger.Logger;
import main.java.com.logger.LoggerFactory;
import main.java.com.logger.level.Level;
import main.java.com.server.handlers.ServiceChat;
import sun.misc.Signal;

import java.io.PrintWriter;
import java.net.PortUnreachableException;
import java.net.ServerSocket;
import java.util.Set;

/**
 * <h1>The MultiThreadedServer object</h1>
 * <p>
 *     This class is the launcher of the multi threaded server
 * </p>
 * //TODO Include diagram of MultiThreadedServer
 *
 * @author Raphael Dray
 * @version 0.0.2
 * @since 0.0.1
 * @see Set
 * @see PrintWriter
 * @see ServerSocket
 */
public class ServerChat {
    private static final Logger logger = LoggerFactory.getLogger(ServerChat.class.getSimpleName());

    /**
     * The default port if not specified is 8080
     */
    private static final int DEFAULT_PORT = 8080;
    private static final int MAX_USERS = 3;
    private int port;
    private String host;

    /**
     * Constructor of the multi threaded server.
     * Initialize the host and the port.
     * @param host The host of the server
     * @param port The port of the server
     */
    public ServerChat(final String host, final int port) {
        this.host = host;
        try {
            if (port > 65536) {
                this.port = DEFAULT_PORT;
                throw new PortUnreachableException("Port higher than 65536");
            }
            this.port = port;
            logger.log("Listening on port " + this.port, Level.INFO);
        } catch (PortUnreachableException e) {
            logger.log(e.getMessage(), Level.ERROR);
        }
    }

    private static void handleSignal(Signal signal) {
        logger.log("Server exiting..", Level.INFO);
        System.exit(0);
    }

    /**
     * The actual port of the server
     * @return int - The port of the server
     */
    public int actualPort() {
        return this.port;
    }

    public static void main(String[] args) throws Exception {
        handleSignals();
        ServerChat server = new ServerChat("localhost", 4444);
        try(ServerSocket listener = new ServerSocket(server.actualPort())) {
            //noinspection InfiniteLoopStatement
            while (true) new Thread(new ServiceChat(listener.accept())).start();
        }
    }

    private static void handleSignals() {
        Signal.handle(new Signal("INT"), ServerChat::handleSignal);
        Signal.handle(new Signal("TERM"), ServerChat::handleSignal);
    }
}