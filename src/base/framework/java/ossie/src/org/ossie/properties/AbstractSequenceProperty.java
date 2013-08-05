package org.ossie.properties;

import java.util.ArrayList;
import java.util.List;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;

abstract class AbstractSequenceProperty<T> extends Property<List<T>> {

    private final String type;

    protected AbstractSequenceProperty(String id, String name, String type, List<T> value,
                                       Mode mode,  Action action, Kind[] kinds) {
        super(id, name, value, mode, action, kinds);
        this.type = type;
    }

    @Override
    public void fromString(String str) {
        throw new IllegalArgumentException("Only simple properties can be initialized with strings");
    }

    @Override
    protected List<T> fromAny_(Any any) {
        if (AnyUtils.isNull(any)) {
            return new ArrayList<T>();
        } else {
            return extract(any);
        }
    }

    @Override
    public Any toAny() {
        Any any = ORB.init().create_any();
        if (this.value != null) {
            insert(any, this.value);
        }
        return any;
    }

    public String getType() {
        return this.type;
    }

    protected abstract List<T> extract(Any any);
    protected abstract void insert(Any any, List<T> value);
}
