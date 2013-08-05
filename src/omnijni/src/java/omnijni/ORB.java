package omnijni;

public abstract class ORB {

    public static org.omg.CORBA.Object string_to_object (String ior)
    {
        long ref = string_to_object_ref(ior);
        return new CORBAObject(ref);
    }

    public static String object_to_string (org.omg.CORBA.Object obj)
    {
        if (obj instanceof omnijni.ObjectImpl) {
            return objectref_to_string(((omnijni.ObjectImpl)obj)._get_object_ref());
        } else {
            org.omg.CORBA.ORB orb = ((org.omg.CORBA.portable.ObjectImpl)obj)._orb();
            return orb.object_to_string(obj);
        }
    }

    static {
        System.loadLibrary("omnijni");
    }

    private static native long string_to_object_ref (String ior);
    private static native String objectref_to_string (long ref);
}