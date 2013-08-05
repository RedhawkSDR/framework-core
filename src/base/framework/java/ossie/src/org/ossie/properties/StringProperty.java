package org.ossie.properties;

import org.omg.CORBA.Any;

public class StringProperty extends AbstractSimpleProperty<String> {
    public StringProperty(String id, String name, String value, Mode mode,
                          Action action, Kind[] kinds) {
        super(id, name, "string", value, mode, action, kinds);
    }

    protected String extract(Any any) {
        return String.valueOf(AnyUtils.convertAny(any));
    }

    protected void insert(Any any, String value) {
        any.insert_string(value);
    }

    protected String parseString(String str) {
        return str;
    }
}
