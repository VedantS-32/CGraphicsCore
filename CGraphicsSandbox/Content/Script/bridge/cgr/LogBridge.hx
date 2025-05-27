package cgr;

@:hlNative("CGraphicsScript")
class LogBridge {
    public static function cgr_trace(message:String):Void {}
    public static function cgr_info(message:String):Void {}
    public static function cgr_warn(message:String):Void {}
    public static function cgr_error(message:String):Void {}
}