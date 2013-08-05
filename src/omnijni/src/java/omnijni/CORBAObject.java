package omnijni;

public class CORBAObject extends omnijni.ObjectImpl
{
    public CORBAObject ()
    {
        super();
    }

    protected CORBAObject (long ref)
    {
        super(ref);
    }

    public String[] _ids() {
        return new String[]{"IDL:omg.org/CORBA/Object:1.0"};
    }

    protected native long _narrow_object_ref (long ref);

    protected long _get_object_ref (long ref)
    {
        return ref;
    }

    static {
        System.loadLibrary("omnijni");
    }
}