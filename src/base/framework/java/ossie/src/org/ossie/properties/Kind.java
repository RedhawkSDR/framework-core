package org.ossie.properties;

public enum Kind {
    ALLOCATION("allocation"),
    CONFIGURE("configure"),
    TEST("test"),
    EXECPARAM("execparam"),
    FACTORYPARAM("factoryparam"),
    EVENT("event"),
    MESSAGE("message");
            
    public String toString() {
        return this.name;
    }

    public static Kind get(String name) {
        for (Kind kind : Kind.values()) {
            if (kind.name.equals(name)) {
                return kind;
            }
        }
        return null;
    }

    public static Kind[] get(String[] kinds) {
        Kind[] retval = new Kind[kinds.length];
        for (int ii = 0; ii < kinds.length; ++ii) {
            retval[ii] = Kind.get(kinds[ii]);
        }
        return retval;
    }

    private Kind (String name) {
        this.name = name;
    }
    private final String name;
}
