package org.ossie.properties;

import java.util.List;

import org.omg.CORBA.Any;
import org.omg.CORBA.TCKind;
import org.omg.CORBA.ORB;

abstract class NumericSequenceProperty<T extends Number> extends AbstractSequenceProperty<T> {
    public NumericSequenceProperty(String id, String name, String type, List<T> value, Mode mode,
                                   Action action, Kind[] kinds) {
        super(id, name, type, value, mode, action, kinds);
    }

    protected List<T> extract(Any any) {
        try {
            return fromNumberArray((Number[])AnyUtils.convertAny(any));
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }
    protected abstract List<T> fromNumberArray(Number[] array);
}
