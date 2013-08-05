package org.ossie.properties;

public class BooleanProperty extends AbstractSimpleProperty<Boolean> {
    public BooleanProperty(String id, String name, Boolean value, Mode mode,
                           Action action, Kind[] kinds) {
        super(id, name, "boolean", value, mode, action, kinds);
    }

    protected Boolean extract(org.omg.CORBA.Any any) {
        try {
            return (Boolean)AnyUtils.convertAny(any);
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }

    protected void insert(org.omg.CORBA.Any any, Boolean value) {
        any.insert_boolean(value);
     }

    protected Boolean parseString(String str) {
        return Boolean.valueOf(str);
    }
}
