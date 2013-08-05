package omnijni;

import org.omg.CORBA.ORB;
import org.omg.CORBA.Any;
import org.omg.CORBA.portable.OutputStream;

public abstract class AnyUtils {

    public static org.omg.CORBA.Any from_bytes (byte[] buffer)
    {
        OutputStream out = ORB.init().create_output_stream();
        out.write_octet_array(buffer, 0, buffer.length);
        return out.create_input_stream().read_any();
    }

    public static byte[] to_bytes (Any any)
    {
        OutputStream out = any.create_output_stream();
        out.write_any(any);
        return ((com.sun.corba.se.impl.encoding.CDROutputStream)out).toByteArray();
    }
}
