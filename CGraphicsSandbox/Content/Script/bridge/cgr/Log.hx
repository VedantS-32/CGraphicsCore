package cgr;

class Log {
    public static inline function info(message:String):Void {
        LogBridge.cgr_info(message);
    }

    public static inline function trace(message:String):Void {
        LogBridge.cgr_trace(message);
    }

    public static inline function warn(message:String):Void {
        LogBridge.cgr_warn(message);
    }

    public static inline function error(message:String):Void {
        LogBridge.cgr_error(message);
    }
}