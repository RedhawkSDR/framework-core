package org.ossie.properties;

public class CharProperty extends AbstractSimpleProperty<Character> {
    public CharProperty(String id, String name, Character value, Mode mode,
                        Action action, Kind[] kinds) {
        super(id, name, "char", value, mode, action, kinds);
    }

    protected Character extract(org.omg.CORBA.Any any) {
        try {
            return (Character)AnyUtils.convertAny(any);
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }

    protected void insert(org.omg.CORBA.Any any, Character value) {
        any.insert_char(value);
     }

    protected Character parseString(String str) {
        return str.charAt(0);
    }
}
