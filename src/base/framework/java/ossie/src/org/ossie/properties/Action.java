package org.ossie.properties;

public enum Action {
    EQ("eq"),
    NE("ne"),
    GT("gt"),
    LT("lt"),
    GE("ge"),
    LE("le"),
    EXTERNAL("external");

    public String toString() {
        return this.name;
    }

    public static Action get(String name) {
        for (Action action : Action.values()) {
            if (action.name.equals(name)) {
                return action;
            }
        }
        return null;
    }

    private Action(String name) {
        this.name = name;
    }
    private String name;
}
