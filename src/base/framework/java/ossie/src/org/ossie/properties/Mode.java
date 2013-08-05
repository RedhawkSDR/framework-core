package org.ossie.properties;

public enum Mode {
    READONLY("readonly"),
    READWRITE("readwrite"),
    WRITEONLY("writeonly");
            
    public String toString() {
        return this.name;
    }

    public static Mode get(String name) {
        for (Mode mode : Mode.values()) {
            if (mode.name.equals(name)) {
                return mode;
            }
        }
        return null;
    }

    private Mode (String name) {
        this.name = name;
    }
    private final String name;
}
