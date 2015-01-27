var Elm = Elm || { Native: {} };
Elm.Background = Elm.Background || {};
Elm.Background.make = function (_elm) {
   "use strict";
   _elm.Background = _elm.Background || {};
   if (_elm.Background.values)
   return _elm.Background.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Background",
   $BackgroundState = Elm.BackgroundState.make(_elm),
   $Basics = Elm.Basics.make(_elm),
   $DeviceMessage = Elm.DeviceMessage.make(_elm),
   $DevicePacket = Elm.DevicePacket.make(_elm),
   $ExtensionMessage = Elm.ExtensionMessage.make(_elm),
   $FromGuiMessage = Elm.FromGuiMessage.make(_elm),
   $Maybe = Elm.Maybe.make(_elm),
   $Signal = Elm.Signal.make(_elm),
   $Time = Elm.Time.make(_elm),
   $ToGuiMessage = Elm.ToGuiMessage.make(_elm);
   var fromExtension = _P.portIn("fromExtension",
   _P.incomingSignal(function (v) {
      return typeof v === "object" && "ping" in v && "getInputs" in v && "update" in v ? {_: {}
                                                                                         ,ping: v.ping === null ? Elm.Maybe.make(_elm).Nothing : Elm.Maybe.make(_elm).Just(typeof v.ping === "object" && v.ping instanceof Array ? {ctor: "_Tuple0"} : _U.badPort("an array",
                                                                                         v.ping))
                                                                                         ,getInputs: v.getInputs === null ? Elm.Maybe.make(_elm).Nothing : Elm.Maybe.make(_elm).Just(typeof v.getInputs === "object" && "context" in v.getInputs ? {_: {}
                                                                                                                                                                                                                                                   ,context: typeof v.getInputs.context === "string" || typeof v.getInputs.context === "object" && v.getInputs.context instanceof String ? v.getInputs.context : _U.badPort("a string",
                                                                                                                                                                                                                                                   v.getInputs.context)} : _U.badPort("an object with fields \'context\'",
                                                                                         v.getInputs))
                                                                                         ,update: v.update === null ? Elm.Maybe.make(_elm).Nothing : Elm.Maybe.make(_elm).Just(typeof v.update === "object" && "context" in v.update && "login" in v.update && "password" in v.update ? {_: {}
                                                                                                                                                                                                                                                                                        ,context: typeof v.update.context === "string" || typeof v.update.context === "object" && v.update.context instanceof String ? v.update.context : _U.badPort("a string",
                                                                                                                                                                                                                                                                                        v.update.context)
                                                                                                                                                                                                                                                                                        ,login: typeof v.update.login === "string" || typeof v.update.login === "object" && v.update.login instanceof String ? v.update.login : _U.badPort("a string",
                                                                                                                                                                                                                                                                                        v.update.login)
                                                                                                                                                                                                                                                                                        ,password: typeof v.update.password === "string" || typeof v.update.password === "object" && v.update.password instanceof String ? v.update.password : _U.badPort("a string",
                                                                                                                                                                                                                                                                                        v.update.password)} : _U.badPort("an object with fields \'context\', \'login\', \'password\'",
                                                                                         v.update))} : _U.badPort("an object with fields \'ping\', \'getInputs\', \'update\'",
      v);
   }));
   var fromDevice = _P.portIn("fromDevice",
   _P.incomingSignal(function (v) {
      return typeof v === "object" && "setHidConnected" in v && "receiveCommand" in v && "appendToLog" in v ? {_: {}
                                                                                                              ,setHidConnected: v.setHidConnected === null ? Elm.Maybe.make(_elm).Nothing : Elm.Maybe.make(_elm).Just(typeof v.setHidConnected === "boolean" ? v.setHidConnected : _U.badPort("a boolean (true or false)",
                                                                                                              v.setHidConnected))
                                                                                                              ,receiveCommand: v.receiveCommand === null ? Elm.Maybe.make(_elm).Nothing : Elm.Maybe.make(_elm).Just(typeof v.receiveCommand === "object" && v.receiveCommand instanceof Array ? Elm.Native.List.make(_elm).fromArray(v.receiveCommand.map(function (v) {
                                                                                                                 return typeof v === "number" ? v : _U.badPort("a number",
                                                                                                                 v);
                                                                                                              })) : _U.badPort("an array",
                                                                                                              v.receiveCommand))
                                                                                                              ,appendToLog: v.appendToLog === null ? Elm.Maybe.make(_elm).Nothing : Elm.Maybe.make(_elm).Just(typeof v.appendToLog === "string" || typeof v.appendToLog === "object" && v.appendToLog instanceof String ? v.appendToLog : _U.badPort("a string",
                                                                                                              v.appendToLog))} : _U.badPort("an object with fields \'setHidConnected\', \'receiveCommand\', \'appendToLog\'",
      v);
   }));
   var fromGUI = _P.portIn("fromGUI",
   _P.incomingSignal(function (v) {
      return typeof v === "object" && "setLog" in v && "getState" in v ? {_: {}
                                                                         ,setLog: v.setLog === null ? Elm.Maybe.make(_elm).Nothing : Elm.Maybe.make(_elm).Just(typeof v.setLog === "object" && v.setLog instanceof Array ? Elm.Native.List.make(_elm).fromArray(v.setLog.map(function (v) {
                                                                            return typeof v === "string" || typeof v === "object" && v instanceof String ? v : _U.badPort("a string",
                                                                            v);
                                                                         })) : _U.badPort("an array",
                                                                         v.setLog))
                                                                         ,getState: v.getState === null ? Elm.Maybe.make(_elm).Nothing : Elm.Maybe.make(_elm).Just(typeof v.getState === "object" && v.getState instanceof Array ? {ctor: "_Tuple0"} : _U.badPort("an array",
                                                                         v.getState))} : _U.badPort("an object with fields \'setLog\', \'getState\'",
      v);
   }));
   var inputActions = $Signal.mergeMany(_L.fromArray([A2($Signal.map,
                                                     $DeviceMessage.decode,
                                                     fromDevice)
                                                     ,A2($Signal.map,
                                                     function ($) {
                                                        return $BackgroundState.CommonAction($FromGuiMessage.decode($));
                                                     },
                                                     fromGUI)
                                                     ,A2($Signal.map,
                                                     $ExtensionMessage.decode,
                                                     fromExtension)]));
   var output = function () {
      var go = F2(function (inputActions,
      _v0) {
         return function () {
            switch (_v0.ctor)
            {case "_Tuple3":
               return function () {
                    var s$ = A2($BackgroundState.update,
                    inputActions,
                    _v0._2);
                    var $ = $DeviceMessage.encode(s$),
                    deviceMessage = $._0,
                    a1 = $._1;
                    var s$$ = A2($BackgroundState.update,
                    a1,
                    s$);
                    var $ = $ExtensionMessage.encode(s$$),
                    extMessage = $._0,
                    a2 = $._1;
                    return {ctor: "_Tuple3"
                           ,_0: deviceMessage
                           ,_1: extMessage
                           ,_2: A2($BackgroundState.update,
                           a2,
                           s$$)};
                 }();}
            _U.badCase($moduleName,
            "between lines 44 and 48");
         }();
      });
      return A3($Signal.foldp,
      go,
      {ctor: "_Tuple3"
      ,_0: $DeviceMessage.emptyToDeviceMessage
      ,_1: $ExtensionMessage.emptyToExtensionMessage
      ,_2: $BackgroundState.$default},
      inputActions);
   }();
   var state = A2($Signal.map,
   function (_v5) {
      return function () {
         switch (_v5.ctor)
         {case "_Tuple3": return _v5._2;}
         _U.badCase($moduleName,
         "on line 39, column 26 to 27");
      }();
   },
   output);
   var toGUI = _P.portOut("toGUI",
   _P.outgoingSignal(function (v) {
      return {setLog: Elm.Native.List.make(_elm).toArray(v.setLog).map(function (v) {
                return v;
             })
             ,setConnected: v.setConnected};
   }),
   A2($Signal.map,
   function ($) {
      return $ToGuiMessage.encode(function (_) {
         return _.common;
      }($));
   },
   state));
   var toDevice = _P.portOut("toDevice",
   _P.outgoingSignal(function (v) {
      return {connect: v.connect.ctor === "Nothing" ? null : []
             ,sendCommand: v.sendCommand.ctor === "Nothing" ? null : Elm.Native.List.make(_elm).toArray(v.sendCommand._0).map(function (v) {
                return v;
             })};
   }),
   $Signal.merge(A2($Signal.map,
   function (_v12) {
      return function () {
         switch (_v12.ctor)
         {case "_Tuple3":
            return _v12._0;}
         _U.badCase($moduleName,
         "on line 30, column 27 to 28");
      }();
   },
   output))(A3($Signal.map2,
   F2(function (_v10,s) {
      return function () {
         return s.deviceConnected ? $DeviceMessage.sendCommand($DevicePacket.AppGetStatus) : _U.replace([["connect"
                                                                                                         ,$Maybe.Just({ctor: "_Tuple0"})]],
         $DeviceMessage.emptyToDeviceMessage);
      }();
   }),
   $Time.every($Time.second),
   state)));
   var toExtension = _P.portOut("toExtension",
   _P.outgoingSignal(function (v) {
      return {connectState: v.connectState.ctor === "Nothing" ? null : {state: v.connectState._0.state
                                                                       ,version: v.connectState._0.version}
             ,credentials: v.credentials.ctor === "Nothing" ? null : {context: v.credentials._0.context
                                                                     ,login: v.credentials._0.login
                                                                     ,password: v.credentials._0.password}
             ,noCredentials: v.noCredentials.ctor === "Nothing" ? null : []
             ,updateComplete: v.updateComplete.ctor === "Nothing" ? null : []};
   }),
   A2($Signal.map,
   function (_v17) {
      return function () {
         switch (_v17.ctor)
         {case "_Tuple3":
            return _v17._1;}
         _U.badCase($moduleName,
         "on line 54, column 37 to 38");
      }();
   },
   output));
   _elm.Background.values = {_op: _op
                            ,state: state
                            ,output: output
                            ,inputActions: inputActions};
   return _elm.Background.values;
};
Elm.BackgroundState = Elm.BackgroundState || {};
Elm.BackgroundState.make = function (_elm) {
   "use strict";
   _elm.BackgroundState = _elm.BackgroundState || {};
   if (_elm.BackgroundState.values)
   return _elm.BackgroundState.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "BackgroundState",
   $Basics = Elm.Basics.make(_elm),
   $Byte = Elm.Byte.make(_elm),
   $CommonState = Elm.CommonState.make(_elm),
   $DevicePacket = Elm.DevicePacket.make(_elm),
   $Maybe = Elm.Maybe.make(_elm),
   $Result = Elm.Result.make(_elm);
   var NoOp = {ctor: "NoOp"};
   var CommonAction = function (a) {
      return {ctor: "CommonAction"
             ,_0: a};
   };
   var appendToLog = function (s) {
      return CommonAction($CommonState.AppendToLog(s));
   };
   var Receive = function (a) {
      return {ctor: "Receive"
             ,_0: a};
   };
   var fromPacket = function (r) {
      return function () {
         switch (r.ctor)
         {case "Err":
            return appendToLog(A2($Basics._op["++"],
              "HID Error: ",
              r._0));
            case "Ok":
            return Receive(r._0);}
         _U.badCase($moduleName,
         "between lines 220 and 222");
      }();
   };
   var SetExtensionRequest = function (a) {
      return {ctor: "SetExtensionRequest"
             ,_0: a};
   };
   var SetExtAwaitingPing = function (a) {
      return {ctor: "SetExtAwaitingPing"
             ,_0: a};
   };
   var SetHidConnected = function (a) {
      return {ctor: "SetHidConnected"
             ,_0: a};
   };
   var extensionRequestToLog = function (d) {
      return function () {
         switch (d.ctor)
         {case "ExtCredentials":
            return $Maybe.Just("credentials retrieved");
            case "ExtNeedsNewContext":
            return $Maybe.Just(A2($Basics._op["++"],
              "> adding new credentials for ",
              d._0.context));
            case "ExtNeedsToWriteLogin":
            return $Maybe.Just(A2($Basics._op["++"],
              "> writing credentials for ",
              d._0.context));
            case "ExtNoCredentials":
            return $Maybe.Just("access denied or no credentials");
            case "ExtNotWritten":
            return $Maybe.Just("access denied");
            case "ExtWantsCredentials":
            return $Maybe.Just(A2($Basics._op["++"],
              "> requesting credentials for ",
              d._0.context));
            case "ExtWriteComplete":
            return $Maybe.Just("credentials written");}
         return $Maybe.Nothing;
      }();
   };
   var NoRequest = {ctor: "NoRequest"};
   var ExtNotWritten = {ctor: "ExtNotWritten"};
   var ExtWriteComplete = function (a) {
      return {ctor: "ExtWriteComplete"
             ,_0: a};
   };
   var ExtNeedsToWritePassword = function (a) {
      return {ctor: "ExtNeedsToWritePassword"
             ,_0: a};
   };
   var ExtNeedsToWriteLogin = function (a) {
      return {ctor: "ExtNeedsToWriteLogin"
             ,_0: a};
   };
   var ExtNeedsNewContext = function (a) {
      return {ctor: "ExtNeedsNewContext"
             ,_0: a};
   };
   var ExtWantsToWrite = function (a) {
      return {ctor: "ExtWantsToWrite"
             ,_0: a};
   };
   var ExtNoCredentials = {ctor: "ExtNoCredentials"};
   var ExtCredentials = function (a) {
      return {ctor: "ExtCredentials"
             ,_0: a};
   };
   var ExtNeedsPassword = function (a) {
      return {ctor: "ExtNeedsPassword"
             ,_0: a};
   };
   var ExtNeedsLogin = function (a) {
      return {ctor: "ExtNeedsLogin"
             ,_0: a};
   };
   var update = F2(function (action,
   s) {
      return function () {
         var updateCommon = function (a) {
            return A2($CommonState.update,
            a,
            s.common);
         };
         return function () {
            switch (action.ctor)
            {case "CommonAction":
               switch (action._0.ctor)
                 {case "SetConnected":
                    return function () {
                         var s$ = _U.replace([["common"
                                              ,updateCommon($CommonState.SetConnected(action._0._0))]],
                         s);
                         return !_U.eq(action._0._0,
                         s.common.connected) ? _U.replace([["common"
                                                           ,A2($CommonState.update,
                                                           $CommonState.AppendToLog($CommonState.connectToLog(action._0._0)),
                                                           s$.common)]
                                                          ,["currentContext",""]
                                                          ,["deviceVersion"
                                                           ,_U.eq(action._0._0,
                                                           $CommonState.NotConnected) ? $Maybe.Nothing : s.deviceVersion]],
                         s$) : s;
                      }();}
                 return _U.replace([["common"
                                    ,updateCommon(action._0)]],
                 s);
               case "NoOp": return s;
               case "Receive":
               switch (action._0.ctor)
                 {case "DeviceAddContext":
                    return _U.replace([["extRequest"
                                       ,function () {
                                          var _v24 = s.extRequest;
                                          switch (_v24.ctor)
                                          {case "ExtNeedsNewContext":
                                             return _U.eq(action._0._0,
                                               $DevicePacket.Done) ? ExtWantsToWrite(_v24._0) : ExtNotWritten;}
                                          return NoRequest;
                                       }()]],
                      s);
                    case "DeviceGetLogin":
                    return function () {
                         switch (action._0._0.ctor)
                         {case "Just":
                            return _U.replace([["extRequest"
                                               ,function () {
                                                  var _v28 = s.extRequest;
                                                  switch (_v28.ctor)
                                                  {case "ExtNeedsLogin":
                                                     return ExtNeedsPassword(_U.insert("login",
                                                       action._0._0._0,
                                                       _v28._0));
                                                     case "ExtWantsCredentials":
                                                     return ExtNeedsPassword(_U.insert("login",
                                                       action._0._0._0,
                                                       _v28._0));}
                                                  return NoRequest;
                                               }()]],
                              s);
                            case "Nothing":
                            return _U.replace([["extRequest"
                                               ,ExtNoCredentials]],
                              s);}
                         _U.badCase($moduleName,
                         "between lines 118 and 128");
                      }();
                    case "DeviceGetPassword":
                    return function () {
                         switch (action._0._0.ctor)
                         {case "Just":
                            return _U.replace([["extRequest"
                                               ,function () {
                                                  var _v33 = s.extRequest;
                                                  switch (_v33.ctor)
                                                  {case "ExtNeedsPassword":
                                                     return ExtCredentials(_U.insert("password",
                                                       action._0._0._0,
                                                       _v33._0));}
                                                  return NoRequest;
                                               }()]],
                              s);
                            case "Nothing":
                            return _U.replace([["extRequest"
                                               ,ExtNoCredentials]],
                              s);}
                         _U.badCase($moduleName,
                         "between lines 128 and 136");
                      }();
                    case "DeviceGetStatus":
                    return A2(update,
                      CommonAction($CommonState.SetConnected(function () {
                         switch (action._0._0.ctor)
                         {case "LockScreen":
                            return $CommonState.NoPin;
                            case "Locked":
                            return $CommonState.NoPin;
                            case "NeedCard":
                            return $CommonState.NoCard;
                            case "Unlocked":
                            return $CommonState.Connected;}
                         _U.badCase($moduleName,
                         "between lines 199 and 204");
                      }())),
                      s);
                    case "DeviceGetVersion":
                    return A2(update,
                      appendToLog(A2($Basics._op["++"],
                      "device is ",
                      A2($Basics._op["++"],
                      action._0._0.version,
                      A2($Basics._op["++"],
                      " ",
                      A2($Basics._op["++"],
                      $Basics.toString(action._0._0.flashMemSize),
                      "MBit"))))),
                      _U.replace([["deviceVersion"
                                  ,$Maybe.Just(action._0._0)]],
                      s));
                    case "DeviceSetContext":
                    return function () {
                         switch (action._0._0.ctor)
                         {case "ContextSet":
                            return function () {
                                 var _v37 = s.extRequest;
                                 switch (_v37.ctor)
                                 {case "ExtNeedsLogin":
                                    return _U.replace([["currentContext"
                                                       ,_v37._0.context]],
                                      s);
                                    case "ExtNeedsPassword":
                                    return _U.replace([["currentContext"
                                                       ,_v37._0.context]],
                                      s);
                                    case "ExtNeedsToWriteLogin":
                                    return _U.replace([["currentContext"
                                                       ,_v37._0.context]],
                                      s);
                                    case "ExtNeedsToWritePassword":
                                    return _U.replace([["currentContext"
                                                       ,_v37._0.context]],
                                      s);
                                    case "ExtWantsCredentials":
                                    return _U.replace([["currentContext"
                                                       ,_v37._0.context]
                                                      ,["extRequest"
                                                       ,ExtNeedsLogin(_v37._0)]],
                                      s);
                                    case "ExtWantsToWrite":
                                    return _U.replace([["currentContext"
                                                       ,_v37._0.context]
                                                      ,["extRequest"
                                                       ,ExtNeedsToWriteLogin(_v37._0)]],
                                      s);}
                                 return s;
                              }();
                            case "NoCardForContext":
                            return A2(update,
                              CommonAction($CommonState.SetConnected($CommonState.NoCard)),
                              s);
                            case "UnknownContext":
                            return function () {
                                 var _v44 = s.extRequest;
                                 switch (_v44.ctor)
                                 {case "ExtNeedsLogin":
                                    return _U.replace([["extRequest"
                                                       ,ExtNoCredentials]],
                                      s);
                                    case "ExtNeedsPassword":
                                    return _U.replace([["extRequest"
                                                       ,ExtNoCredentials]],
                                      s);
                                    case "ExtNeedsToWriteLogin":
                                    return _U.replace([["extRequest"
                                                       ,ExtNotWritten]],
                                      s);
                                    case "ExtNeedsToWritePassword":
                                    return _U.replace([["extRequest"
                                                       ,ExtNotWritten]],
                                      s);
                                    case "ExtWantsCredentials":
                                    return _U.replace([["extRequest"
                                                       ,ExtNoCredentials]],
                                      s);
                                    case "ExtWantsToWrite":
                                    return _U.replace([["extRequest"
                                                       ,ExtNeedsNewContext(_v44._0)]],
                                      s);}
                                 return s;
                              }();}
                         _U.badCase($moduleName,
                         "between lines 153 and 190");
                      }();
                    case "DeviceSetLogin":
                    return _U.replace([["extRequest"
                                       ,function () {
                                          var _v51 = s.extRequest;
                                          switch (_v51.ctor)
                                          {case "ExtNeedsToWriteLogin":
                                             return _U.eq(action._0._0,
                                               $DevicePacket.Done) ? ExtNeedsToWritePassword(_U.remove("login",
                                               _v51._0)) : ExtNotWritten;}
                                          return NoRequest;
                                       }()]],
                      s);
                    case "DeviceSetPassword":
                    return _U.replace([["extRequest"
                                       ,function () {
                                          var _v53 = s.extRequest;
                                          switch (_v53.ctor)
                                          {case "ExtNeedsToWritePassword":
                                             return _U.eq(action._0._0,
                                               $DevicePacket.Done) ? ExtWriteComplete(_U.remove("password",
                                               _v53._0)) : ExtNotWritten;}
                                          return NoRequest;
                                       }()]],
                      s);}
                 return A2(update,
                 appendToLog(A2($Basics._op["++"],
                 "Error: received unhandled packet ",
                 $Basics.toString(action._0))),
                 s);
               case "SetExtAwaitingPing":
               return _U.replace([["extAwaitingPing"
                                  ,action._0]],
                 s);
               case "SetExtensionRequest":
               return _U.replace([["extRequest"
                                  ,action._0]],
                 s);
               case "SetHidConnected":
               return $Basics.not(action._0) ? A2(update,
                 CommonAction($CommonState.SetConnected($CommonState.NotConnected)),
                 _U.replace([["deviceConnected"
                             ,false]],
                 s)) : _U.replace([["deviceConnected"
                                   ,true]],
                 s);}
            _U.badCase($moduleName,
            "between lines 95 and 217");
         }();
      }();
   });
   var ExtWantsCredentials = function (a) {
      return {ctor: "ExtWantsCredentials"
             ,_0: a};
   };
   var $default = {_: {}
                  ,common: $CommonState.$default
                  ,currentContext: ""
                  ,deviceConnected: false
                  ,deviceVersion: $Maybe.Nothing
                  ,extAwaitingPing: false
                  ,extRequest: NoRequest};
   var BackgroundState = F6(function (a,
   b,
   c,
   d,
   e,
   f) {
      return {_: {}
             ,common: f
             ,currentContext: c
             ,deviceConnected: a
             ,deviceVersion: b
             ,extAwaitingPing: d
             ,extRequest: e};
   });
   _elm.BackgroundState.values = {_op: _op
                                 ,BackgroundState: BackgroundState
                                 ,$default: $default
                                 ,ExtWantsCredentials: ExtWantsCredentials
                                 ,ExtNeedsLogin: ExtNeedsLogin
                                 ,ExtNeedsPassword: ExtNeedsPassword
                                 ,ExtCredentials: ExtCredentials
                                 ,ExtNoCredentials: ExtNoCredentials
                                 ,ExtWantsToWrite: ExtWantsToWrite
                                 ,ExtNeedsNewContext: ExtNeedsNewContext
                                 ,ExtNeedsToWriteLogin: ExtNeedsToWriteLogin
                                 ,ExtNeedsToWritePassword: ExtNeedsToWritePassword
                                 ,ExtWriteComplete: ExtWriteComplete
                                 ,ExtNotWritten: ExtNotWritten
                                 ,NoRequest: NoRequest
                                 ,extensionRequestToLog: extensionRequestToLog
                                 ,SetHidConnected: SetHidConnected
                                 ,SetExtAwaitingPing: SetExtAwaitingPing
                                 ,SetExtensionRequest: SetExtensionRequest
                                 ,Receive: Receive
                                 ,CommonAction: CommonAction
                                 ,NoOp: NoOp
                                 ,update: update
                                 ,fromPacket: fromPacket
                                 ,appendToLog: appendToLog};
   return _elm.BackgroundState.values;
};
Elm.Basics = Elm.Basics || {};
Elm.Basics.make = function (_elm) {
   "use strict";
   _elm.Basics = _elm.Basics || {};
   if (_elm.Basics.values)
   return _elm.Basics.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Basics",
   $Native$Basics = Elm.Native.Basics.make(_elm),
   $Native$Show = Elm.Native.Show.make(_elm),
   $Native$Utils = Elm.Native.Utils.make(_elm);
   var uncurry = F2(function (f,
   _v0) {
      return function () {
         switch (_v0.ctor)
         {case "_Tuple2": return A2(f,
              _v0._0,
              _v0._1);}
         _U.badCase($moduleName,
         "on line 460, column 19 to 24");
      }();
   });
   var curry = F3(function (f,
   a,
   b) {
      return f({ctor: "_Tuple2"
               ,_0: a
               ,_1: b});
   });
   var flip = F3(function (f,b,a) {
      return A2(f,a,b);
   });
   var snd = function (_v4) {
      return function () {
         switch (_v4.ctor)
         {case "_Tuple2": return _v4._1;}
         _U.badCase($moduleName,
         "on line 444, column 13 to 14");
      }();
   };
   var fst = function (_v8) {
      return function () {
         switch (_v8.ctor)
         {case "_Tuple2": return _v8._0;}
         _U.badCase($moduleName,
         "on line 440, column 13 to 14");
      }();
   };
   var always = F2(function (a,
   _v12) {
      return function () {
         return a;
      }();
   });
   var identity = function (x) {
      return x;
   };
   _op["<|"] = F2(function (f,x) {
      return f(x);
   });
   _op["|>"] = F2(function (x,f) {
      return f(x);
   });
   _op[">>"] = F3(function (f,
   g,
   x) {
      return g(f(x));
   });
   _op["<<"] = F3(function (g,
   f,
   x) {
      return g(f(x));
   });
   _op["++"] = $Native$Utils.append;
   var toString = $Native$Show.toString;
   var isInfinite = $Native$Basics.isInfinite;
   var isNaN = $Native$Basics.isNaN;
   var toFloat = $Native$Basics.toFloat;
   var ceiling = $Native$Basics.ceiling;
   var floor = $Native$Basics.floor;
   var truncate = $Native$Basics.truncate;
   var round = $Native$Basics.round;
   var otherwise = true;
   var not = $Native$Basics.not;
   var xor = $Native$Basics.xor;
   _op["||"] = $Native$Basics.or;
   _op["&&"] = $Native$Basics.and;
   var max = $Native$Basics.max;
   var min = $Native$Basics.min;
   var GT = {ctor: "GT"};
   var EQ = {ctor: "EQ"};
   var LT = {ctor: "LT"};
   var compare = $Native$Basics.compare;
   _op[">="] = $Native$Basics.ge;
   _op["<="] = $Native$Basics.le;
   _op[">"] = $Native$Basics.gt;
   _op["<"] = $Native$Basics.lt;
   _op["/="] = $Native$Basics.neq;
   _op["=="] = $Native$Basics.eq;
   var e = $Native$Basics.e;
   var pi = $Native$Basics.pi;
   var clamp = $Native$Basics.clamp;
   var logBase = $Native$Basics.logBase;
   var abs = $Native$Basics.abs;
   var negate = $Native$Basics.negate;
   var sqrt = $Native$Basics.sqrt;
   var atan2 = $Native$Basics.atan2;
   var atan = $Native$Basics.atan;
   var asin = $Native$Basics.asin;
   var acos = $Native$Basics.acos;
   var tan = $Native$Basics.tan;
   var sin = $Native$Basics.sin;
   var cos = $Native$Basics.cos;
   _op["^"] = $Native$Basics.exp;
   _op["%"] = $Native$Basics.mod;
   var rem = $Native$Basics.rem;
   _op["//"] = $Native$Basics.div;
   _op["/"] = $Native$Basics.floatDiv;
   _op["*"] = $Native$Basics.mul;
   _op["-"] = $Native$Basics.sub;
   _op["+"] = $Native$Basics.add;
   var toPolar = $Native$Basics.toPolar;
   var fromPolar = $Native$Basics.fromPolar;
   var turns = $Native$Basics.turns;
   var degrees = $Native$Basics.degrees;
   var radians = function (t) {
      return t;
   };
   _elm.Basics.values = {_op: _op
                        ,radians: radians
                        ,degrees: degrees
                        ,turns: turns
                        ,fromPolar: fromPolar
                        ,toPolar: toPolar
                        ,rem: rem
                        ,cos: cos
                        ,sin: sin
                        ,tan: tan
                        ,acos: acos
                        ,asin: asin
                        ,atan: atan
                        ,atan2: atan2
                        ,sqrt: sqrt
                        ,negate: negate
                        ,abs: abs
                        ,logBase: logBase
                        ,clamp: clamp
                        ,pi: pi
                        ,e: e
                        ,compare: compare
                        ,LT: LT
                        ,EQ: EQ
                        ,GT: GT
                        ,min: min
                        ,max: max
                        ,xor: xor
                        ,not: not
                        ,otherwise: otherwise
                        ,round: round
                        ,truncate: truncate
                        ,floor: floor
                        ,ceiling: ceiling
                        ,toFloat: toFloat
                        ,isNaN: isNaN
                        ,isInfinite: isInfinite
                        ,toString: toString
                        ,identity: identity
                        ,always: always
                        ,fst: fst
                        ,snd: snd
                        ,flip: flip
                        ,curry: curry
                        ,uncurry: uncurry};
   return _elm.Basics.values;
};
Elm.Bitwise = Elm.Bitwise || {};
Elm.Bitwise.make = function (_elm) {
   "use strict";
   _elm.Bitwise = _elm.Bitwise || {};
   if (_elm.Bitwise.values)
   return _elm.Bitwise.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Bitwise",
   $Native$Bitwise = Elm.Native.Bitwise.make(_elm);
   var shiftRightLogical = $Native$Bitwise.shiftRightLogical;
   var shiftRight = $Native$Bitwise.shiftRightArithmatic;
   var shiftLeft = $Native$Bitwise.shiftLeft;
   var complement = $Native$Bitwise.complement;
   var xor = $Native$Bitwise.xor;
   var or = $Native$Bitwise.or;
   var and = $Native$Bitwise.and;
   _elm.Bitwise.values = {_op: _op
                         ,and: and
                         ,or: or
                         ,xor: xor
                         ,complement: complement
                         ,shiftLeft: shiftLeft
                         ,shiftRight: shiftRight
                         ,shiftRightLogical: shiftRightLogical};
   return _elm.Bitwise.values;
};
Elm.Byte = Elm.Byte || {};
Elm.Byte.make = function (_elm) {
   "use strict";
   _elm.Byte = _elm.Byte || {};
   if (_elm.Byte.values)
   return _elm.Byte.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Byte",
   $Basics = Elm.Basics.make(_elm),
   $Char = Elm.Char.make(_elm),
   $List = Elm.List.make(_elm),
   $Result = Elm.Result.make(_elm),
   $String = Elm.String.make(_elm);
   var toByteString = F2(function (size,
   ints) {
      return _U.cmp(size,
      $List.length(ints)) > 0 || _U.cmp(size,
      0) < 1 ? $Result.Err("Invalid size to convert to bytestring") : A3($List.foldr,
      F2(function ($int,b) {
         return b && (_U.cmp($int,
         0) > -1 && _U.cmp($int,
         256) < 0);
      }),
      true,
      A2($List.take,
      size,
      ints)) ? $Result.Ok($String.fromList(A2($List.map,
      $Char.fromCode,
      A2($List.take,
      size,
      ints)))) : $Result.Err("Invalid char given to byte conversion (unicode?)");
   });
   var byteString = function (s) {
      return toByteString($String.length(s))(A2($List.map,
      $Char.toCode,
      $String.toList(s)));
   };
   var toByte = function (x) {
      return _U.cmp(x,
      0) > -1 && _U.cmp(x,
      256) < 0 ? $Result.Ok(x) : $Result.Err("Invalid int given to byte conversion");
   };
   _elm.Byte.values = {_op: _op
                      ,toByte: toByte
                      ,toByteString: toByteString
                      ,byteString: byteString};
   return _elm.Byte.values;
};
Elm.Char = Elm.Char || {};
Elm.Char.make = function (_elm) {
   "use strict";
   _elm.Char = _elm.Char || {};
   if (_elm.Char.values)
   return _elm.Char.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Char",
   $Native$Char = Elm.Native.Char.make(_elm);
   var fromCode = $Native$Char.fromCode;
   var toCode = $Native$Char.toCode;
   var toLocaleLower = $Native$Char.toLocaleLower;
   var toLocaleUpper = $Native$Char.toLocaleUpper;
   var toLower = $Native$Char.toLower;
   var toUpper = $Native$Char.toUpper;
   var isHexDigit = $Native$Char.isHexDigit;
   var isOctDigit = $Native$Char.isOctDigit;
   var isDigit = $Native$Char.isDigit;
   var isLower = $Native$Char.isLower;
   var isUpper = $Native$Char.isUpper;
   _elm.Char.values = {_op: _op
                      ,isUpper: isUpper
                      ,isLower: isLower
                      ,isDigit: isDigit
                      ,isOctDigit: isOctDigit
                      ,isHexDigit: isHexDigit
                      ,toUpper: toUpper
                      ,toLower: toLower
                      ,toLocaleUpper: toLocaleUpper
                      ,toLocaleLower: toLocaleLower
                      ,toCode: toCode
                      ,fromCode: fromCode};
   return _elm.Char.values;
};
Elm.CommonState = Elm.CommonState || {};
Elm.CommonState.make = function (_elm) {
   "use strict";
   _elm.CommonState = _elm.CommonState || {};
   if (_elm.CommonState.values)
   return _elm.CommonState.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "CommonState",
   $List = Elm.List.make(_elm);
   var update = F2(function (action,
   s) {
      return function () {
         switch (action.ctor)
         {case "AppendToLog":
            return _U.replace([["log"
                               ,A2($List._op["::"],
                               action._0,
                               s.log)]],
              s);
            case "CommonNoOp": return s;
            case "GetState": return s;
            case "SetConnected":
            return _U.replace([["connected"
                               ,action._0]],
              s);
            case "SetLog":
            return _U.replace([["log"
                               ,action._0]],
              s);}
         _U.badCase($moduleName,
         "between lines 38 and 43");
      }();
   });
   var apply = F2(function (actions,
   state) {
      return A3($List.foldr,
      update,
      state,
      actions);
   });
   var CommonNoOp = {ctor: "CommonNoOp"};
   var GetState = {ctor: "GetState"};
   var AppendToLog = function (a) {
      return {ctor: "AppendToLog"
             ,_0: a};
   };
   var SetConnected = function (a) {
      return {ctor: "SetConnected"
             ,_0: a};
   };
   var SetLog = function (a) {
      return {ctor: "SetLog"
             ,_0: a};
   };
   var connectToLog = function (c) {
      return function () {
         switch (c.ctor)
         {case "Connected":
            return "device status: unlocked";
            case "NoCard":
            return "device status: no card present";
            case "NoPin":
            return "device status: locked";
            case "NotConnected":
            return "device disconnected";}
         _U.badCase($moduleName,
         "between lines 22 and 28");
      }();
   };
   var NoPin = {ctor: "NoPin"};
   var NoCard = {ctor: "NoCard"};
   var Connected = {ctor: "Connected"};
   var NotConnected = {ctor: "NotConnected"};
   var $default = {_: {}
                  ,connected: NotConnected
                  ,log: _L.fromArray([])};
   var CommonState = F2(function (a,
   b) {
      return {_: {}
             ,connected: a
             ,log: b};
   });
   _elm.CommonState.values = {_op: _op
                             ,CommonState: CommonState
                             ,$default: $default
                             ,NotConnected: NotConnected
                             ,Connected: Connected
                             ,NoCard: NoCard
                             ,NoPin: NoPin
                             ,connectToLog: connectToLog
                             ,SetLog: SetLog
                             ,SetConnected: SetConnected
                             ,AppendToLog: AppendToLog
                             ,GetState: GetState
                             ,CommonNoOp: CommonNoOp
                             ,update: update
                             ,apply: apply};
   return _elm.CommonState.values;
};
Elm.DeviceMessage = Elm.DeviceMessage || {};
Elm.DeviceMessage.make = function (_elm) {
   "use strict";
   _elm.DeviceMessage = _elm.DeviceMessage || {};
   if (_elm.DeviceMessage.values)
   return _elm.DeviceMessage.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "DeviceMessage",
   $BackgroundState = Elm.BackgroundState.make(_elm),
   $Basics = Elm.Basics.make(_elm),
   $CommonState = Elm.CommonState.make(_elm),
   $DevicePacket = Elm.DevicePacket.make(_elm),
   $Maybe = Elm.Maybe.make(_elm);
   var toPacket = function (s) {
      return function () {
         var cc = s.currentContext;
         return function () {
            var _v0 = s.extRequest;
            switch (_v0.ctor)
            {case "ExtNeedsLogin":
               return _U.eq(cc,
                 _v0._0.context) ? $Maybe.Just($DevicePacket.AppGetLogin) : $Maybe.Just($DevicePacket.AppSetContext(_v0._0.context));
               case "ExtNeedsNewContext":
               return $Maybe.Just($DevicePacket.AppAddContext(_v0._0.context));
               case "ExtNeedsPassword":
               return _U.eq(cc,
                 _v0._0.context) ? $Maybe.Just($DevicePacket.AppGetPassword) : $Maybe.Just($DevicePacket.AppSetContext(_v0._0.context));
               case "ExtNeedsToWriteLogin":
               return _U.eq(cc,
                 _v0._0.context) ? $Maybe.Just($DevicePacket.AppSetLogin(_v0._0.login)) : $Maybe.Just($DevicePacket.AppSetContext(_v0._0.context));
               case "ExtNeedsToWritePassword":
               return _U.eq(cc,
                 _v0._0.context) ? $Maybe.Just($DevicePacket.AppSetPassword(_v0._0.password)) : $Maybe.Just($DevicePacket.AppSetContext(_v0._0.context));
               case "ExtWantsCredentials":
               return _U.eq(cc,
                 _v0._0.context) ? $Maybe.Just($DevicePacket.AppGetLogin) : $Maybe.Just($DevicePacket.AppSetContext(_v0._0.context));
               case "ExtWantsToWrite":
               return _U.eq(cc,
                 _v0._0.context) ? $Maybe.Just($DevicePacket.AppSetLogin(_v0._0.login)) : $Maybe.Just($DevicePacket.AppSetContext(_v0._0.context));}
            return $Maybe.Nothing;
         }();
      }();
   };
   var decode = function (message) {
      return function () {
         var decode$ = function (_v8) {
            return function () {
               return $Maybe.oneOf(_L.fromArray([A2($Maybe.map,
                                                function ($) {
                                                   return $BackgroundState.CommonAction($CommonState.AppendToLog($));
                                                },
                                                _v8.appendToLog)
                                                ,A2($Maybe.map,
                                                $BackgroundState.SetHidConnected,
                                                _v8.setHidConnected)
                                                ,A2($Maybe.map,
                                                function ($) {
                                                   return $BackgroundState.fromPacket($DevicePacket.fromInts($));
                                                },
                                                _v8.receiveCommand)]));
            }();
         };
         return A2($Maybe.withDefault,
         $BackgroundState.NoOp,
         decode$(message));
      }();
   };
   var emptyToDeviceMessage = {_: {}
                              ,connect: $Maybe.Nothing
                              ,sendCommand: $Maybe.Nothing};
   var encode = function (s) {
      return function () {
         var e = emptyToDeviceMessage;
         return $Basics.not(s.deviceConnected) ? {ctor: "_Tuple2"
                                                 ,_0: _U.replace([["connect"
                                                                  ,$Maybe.Just({ctor: "_Tuple0"})]],
                                                 e)
                                                 ,_1: $BackgroundState.NoOp} : !_U.eq(s.common.connected,
         $CommonState.Connected) ? {ctor: "_Tuple2"
                                   ,_0: e
                                   ,_1: $BackgroundState.NoOp} : _U.eq(s.deviceVersion,
         $Maybe.Nothing) ? {ctor: "_Tuple2"
                           ,_0: _U.replace([["sendCommand"
                                            ,$Maybe.Just($DevicePacket.toInts($DevicePacket.AppGetVersion))]],
                           e)
                           ,_1: $BackgroundState.NoOp} : !_U.eq(s.extRequest,
         $BackgroundState.NoRequest) ? {ctor: "_Tuple2"
                                       ,_0: _U.replace([["sendCommand"
                                                        ,A2($Maybe.map,
                                                        $DevicePacket.toInts,
                                                        toPacket(s))]],
                                       e)
                                       ,_1: A2($Maybe.withDefault,
                                       $BackgroundState.NoOp,
                                       A2($Maybe.map,
                                       function ($) {
                                          return $BackgroundState.CommonAction($CommonState.AppendToLog($));
                                       },
                                       $BackgroundState.extensionRequestToLog(s.extRequest)))} : {ctor: "_Tuple2"
                                                                                                 ,_0: e
                                                                                                 ,_1: $BackgroundState.NoOp};
      }();
   };
   var sendCommand = function (p) {
      return _U.replace([["sendCommand"
                         ,$Maybe.Just($DevicePacket.toInts(p))]],
      emptyToDeviceMessage);
   };
   var ToDeviceMessage = F2(function (a,
   b) {
      return {_: {}
             ,connect: a
             ,sendCommand: b};
   });
   var FromDeviceMessage = F3(function (a,
   b,
   c) {
      return {_: {}
             ,appendToLog: c
             ,receiveCommand: b
             ,setHidConnected: a};
   });
   _elm.DeviceMessage.values = {_op: _op
                               ,FromDeviceMessage: FromDeviceMessage
                               ,ToDeviceMessage: ToDeviceMessage
                               ,sendCommand: sendCommand
                               ,emptyToDeviceMessage: emptyToDeviceMessage
                               ,decode: decode
                               ,encode: encode
                               ,toPacket: toPacket};
   return _elm.DeviceMessage.values;
};
Elm.DevicePacket = Elm.DevicePacket || {};
Elm.DevicePacket.make = function (_elm) {
   "use strict";
   _elm.DevicePacket = _elm.DevicePacket || {};
   if (_elm.DevicePacket.values)
   return _elm.DevicePacket.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "DevicePacket",
   $Basics = Elm.Basics.make(_elm),
   $Bitwise = Elm.Bitwise.make(_elm),
   $Byte = Elm.Byte.make(_elm),
   $Char = Elm.Char.make(_elm),
   $List = Elm.List.make(_elm),
   $Maybe = Elm.Maybe.make(_elm),
   $Result = Elm.Result.make(_elm),
   $String = Elm.String.make(_elm);
   var toInts = function (msg) {
      return function () {
         var param = function (p) {
            return function () {
               switch (p.ctor)
               {case "KeyboardLayout":
                  return 1;
                  case "LockTimeout": return 4;
                  case "LockTimeoutEnable":
                  return 3;
                  case "OfflineMode": return 8;
                  case "TouchDi": return 5;
                  case "TouchProxOs": return 7;
                  case "TouchWheelOs": return 6;
                  case "UserInitKey": return 0;
                  case "UserInterTimeout":
                  return 2;}
               _U.badCase($moduleName,
               "between lines 182 and 192");
            }();
         };
         var stringToInts = function (s) {
            return A2($List.map,
            $Char.toCode,
            $String.toList(s));
         };
         var zeroSize = function (msgType) {
            return _L.fromArray([0
                                ,msgType]);
         };
         var byteStringNull = F2(function (msgType,
         s) {
            return A2($List._op["::"],
            $String.length(s) + 1,
            A2($List._op["::"],
            msgType,
            A2($Basics._op["++"],
            stringToInts(s),
            _L.fromArray([0]))));
         });
         var byteString = F2(function (msgType,
         s) {
            return A2($List._op["::"],
            $String.length(s),
            A2($List._op["::"],
            msgType,
            stringToInts(s)));
         });
         return function () {
            switch (msg.ctor)
            {case "AppAddContext":
               return A2(byteStringNull,
                 10,
                 msg._0);
               case "AppAddCpzCtr":
               return A2($List._op["::"],
                 24,
                 A2($List._op["::"],
                 90,
                 A2($Basics._op["++"],
                 stringToInts(msg._0.cpz),
                 stringToInts(msg._0.ctrNonce))));
               case "AppAddNewCard":
               return zeroSize(104);
               case "AppCheckPassword":
               return zeroSize(9);
               case "AppDebug":
               return A2(byteString,1,msg._0);
               case "AppExportEeprom":
               return zeroSize(53);
               case "AppExportEepromEnd":
               return zeroSize(54);
               case "AppExportEepromStart":
               return zeroSize(70);
               case "AppExportFlash":
               return zeroSize(48);
               case "AppExportFlashEnd":
               return zeroSize(49);
               case "AppExportFlashStart":
               return zeroSize(69);
               case "AppGetCardLogin":
               return zeroSize(97);
               case "AppGetCardPassword":
               return zeroSize(98);
               case "AppGetCpzCtrValues":
               return zeroSize(91);
               case "AppGetCtrValue":
               return zeroSize(103);
               case "AppGetFavorite":
               return _L.fromArray([1
                                   ,95
                                   ,msg._0]);
               case "AppGetFreeSlotAddress":
               return zeroSize(101);
               case "AppGetLogin":
               return zeroSize(5);
               case "AppGetParameter":
               return _L.fromArray([1
                                   ,94
                                   ,param(msg._0)]);
               case "AppGetPassword":
               return zeroSize(6);
               case "AppGetRandomNumber":
               return zeroSize(75);
               case "AppGetStartingParent":
               return zeroSize(102);
               case "AppGetStatus":
               return zeroSize(112);
               case "AppGetVersion":
               return zeroSize(3);
               case "AppImportEeprom":
               return A2(byteString,56,msg._0);
               case "AppImportEepromEnd":
               return zeroSize(57);
               case "AppImportEepromStart":
               return zeroSize(55);
               case "AppImportFlash":
               return A2(byteString,51,msg._0);
               case "AppImportFlashEnd":
               return zeroSize(52);
               case "AppImportFlashStart":
               return _L.fromArray([1
                                   ,50
                                   ,function () {
                                      switch (msg._0.ctor)
                                      {case "FlashGraphicsSpace":
                                         return 1;
                                         case "FlashUserSpace":
                                         return 0;}
                                      _U.badCase($moduleName,
                                      "between lines 207 and 210");
                                   }()]);
               case "AppImportMedia":
               return A2(byteString,83,msg._0);
               case "AppImportMediaEnd":
               return zeroSize(84);
               case "AppImportMediaStart":
               return zeroSize(82);
               case "AppMemoryManageModeEnd":
               return zeroSize(81);
               case "AppMemoryManageModeStart":
               return zeroSize(80);
               case "AppPing":
               return zeroSize(2);
               case "AppReadFlashNode":
               switch (msg._0.ctor)
                 {case "_Tuple2":
                    return _L.fromArray([2
                                        ,85
                                        ,msg._0._0
                                        ,msg._0._1]);}
                 break;
               case "AppResetCard":
               switch (msg._0.ctor)
                 {case "_Tuple2":
                    return _L.fromArray([2
                                        ,96
                                        ,msg._0._0
                                        ,msg._0._1]);}
                 break;
               case "AppSetCardLogin":
               return A2(byteStringNull,
                 99,
                 msg._0);
               case "AppSetCardPassword":
               return A2(byteStringNull,
                 100,
                 msg._0);
               case "AppSetContext":
               return A2(byteStringNull,
                 4,
                 msg._0);
               case "AppSetCtrValue":
               switch (msg._0.ctor)
                 {case "_Tuple3":
                    return _L.fromArray([3
                                        ,89
                                        ,msg._0._0
                                        ,msg._0._1
                                        ,msg._0._2]);}
                 break;
               case "AppSetFavorite":
               switch (msg._1.ctor)
                 {case "_Tuple2":
                    switch (msg._2.ctor)
                      {case "_Tuple2":
                         return A2($List._op["::"],
                           $String.length(msg._3) + 5,
                           A2($List._op["::"],
                           87,
                           A2($List._op["::"],
                           msg._0,
                           A2($List._op["::"],
                           msg._1._0,
                           A2($List._op["::"],
                           msg._1._1,
                           A2($List._op["::"],
                           msg._2._0,
                           A2($List._op["::"],
                           msg._2._1,
                           stringToInts(msg._3))))))));}
                      break;}
                 break;
               case "AppSetLogin":
               return A2(byteStringNull,
                 7,
                 msg._0);
               case "AppSetParameter":
               return _L.fromArray([2
                                   ,93
                                   ,param(msg._0)
                                   ,msg._1]);
               case "AppSetPassword":
               return A2(byteStringNull,
                 8,
                 msg._0);
               case "AppSetStartingParent":
               switch (msg._0.ctor)
                 {case "_Tuple2":
                    return _L.fromArray([2
                                        ,88
                                        ,msg._0._0
                                        ,msg._0._1]);}
                 break;
               case "AppWriteFlashNode":
               switch (msg._0.ctor)
                 {case "_Tuple2":
                    return A2($List._op["::"],
                      $String.length(msg._2) + 3,
                      A2($List._op["::"],
                      86,
                      A2($List._op["::"],
                      msg._0._0,
                      A2($List._op["::"],
                      msg._0._1,
                      A2($List._op["::"],
                      msg._1,
                      stringToInts(msg._2))))));}
                 break;}
            _U.badCase($moduleName,
            "between lines 192 and 250");
         }();
      }();
   };
   var FlashGraphicsSpace = {ctor: "FlashGraphicsSpace"};
   var FlashUserSpace = {ctor: "FlashUserSpace"};
   var OfflineMode = {ctor: "OfflineMode"};
   var TouchProxOs = {ctor: "TouchProxOs"};
   var TouchWheelOs = {ctor: "TouchWheelOs"};
   var TouchDi = {ctor: "TouchDi"};
   var LockTimeout = {ctor: "LockTimeout"};
   var LockTimeoutEnable = {ctor: "LockTimeoutEnable"};
   var UserInterTimeout = {ctor: "UserInterTimeout"};
   var KeyboardLayout = {ctor: "KeyboardLayout"};
   var UserInitKey = {ctor: "UserInitKey"};
   var NotDone = {ctor: "NotDone"};
   var Done = {ctor: "Done"};
   var Unlocked = {ctor: "Unlocked"};
   var LockScreen = {ctor: "LockScreen"};
   var Locked = {ctor: "Locked"};
   var NeedCard = {ctor: "NeedCard"};
   var NoCardForContext = {ctor: "NoCardForContext"};
   var ContextSet = {ctor: "ContextSet"};
   var UnknownContext = {ctor: "UnknownContext"};
   var RequestBlocked = {ctor: "RequestBlocked"};
   var Correct = {ctor: "Correct"};
   var Incorrect = {ctor: "Incorrect"};
   var CpzCtrLutEntry = F2(function (a,
   b) {
      return {_: {}
             ,cpz: a
             ,ctrNonce: b};
   });
   var MpVersion = F2(function (a,
   b) {
      return {_: {}
             ,flashMemSize: a
             ,version: b};
   });
   var DeviceGetStatus = function (a) {
      return {ctor: "DeviceGetStatus"
             ,_0: a};
   };
   var DeviceAddNewCard = function (a) {
      return {ctor: "DeviceAddNewCard"
             ,_0: a};
   };
   var DeviceGetCtrValue = function (a) {
      return {ctor: "DeviceGetCtrValue"
             ,_0: a};
   };
   var DeviceGetStartingParent = function (a) {
      return {ctor: "DeviceGetStartingParent"
             ,_0: a};
   };
   var DeviceGetFreeSlotAddr = function (a) {
      return {ctor: "DeviceGetFreeSlotAddr"
             ,_0: a};
   };
   var DeviceSetCardPassword = function (a) {
      return {ctor: "DeviceSetCardPassword"
             ,_0: a};
   };
   var DeviceSetCardLogin = function (a) {
      return {ctor: "DeviceSetCardLogin"
             ,_0: a};
   };
   var DeviceGetCardPassword = function (a) {
      return {ctor: "DeviceGetCardPassword"
             ,_0: a};
   };
   var DeviceGetCardLogin = function (a) {
      return {ctor: "DeviceGetCardLogin"
             ,_0: a};
   };
   var DeviceResetCard = function (a) {
      return {ctor: "DeviceResetCard"
             ,_0: a};
   };
   var DeviceGetFavorite = function (a) {
      return {ctor: "DeviceGetFavorite"
             ,_0: a};
   };
   var DeviceGetParameter = function (a) {
      return {ctor: "DeviceGetParameter"
             ,_0: a};
   };
   var DeviceSetParameter = function (a) {
      return {ctor: "DeviceSetParameter"
             ,_0: a};
   };
   var DeviceCpzCtrPacketExport = function (a) {
      return {ctor: "DeviceCpzCtrPacketExport"
             ,_0: a};
   };
   var DeviceGetCpzCtrValues = function (a) {
      return {ctor: "DeviceGetCpzCtrValues"
             ,_0: a};
   };
   var DeviceAddCpzCtr = function (a) {
      return {ctor: "DeviceAddCpzCtr"
             ,_0: a};
   };
   var DeviceSetCtrValue = function (a) {
      return {ctor: "DeviceSetCtrValue"
             ,_0: a};
   };
   var DeviceSetStartingParent = function (a) {
      return {ctor: "DeviceSetStartingParent"
             ,_0: a};
   };
   var DeviceSetFavorite = function (a) {
      return {ctor: "DeviceSetFavorite"
             ,_0: a};
   };
   var DeviceWriteFlashNode = function (a) {
      return {ctor: "DeviceWriteFlashNode"
             ,_0: a};
   };
   var DeviceReadFlashNode = function (a) {
      return {ctor: "DeviceReadFlashNode"
             ,_0: a};
   };
   var DeviceImportMedia = function (a) {
      return {ctor: "DeviceImportMedia"
             ,_0: a};
   };
   var DeviceImportMediaEnd = function (a) {
      return {ctor: "DeviceImportMediaEnd"
             ,_0: a};
   };
   var DeviceImportMediaStart = function (a) {
      return {ctor: "DeviceImportMediaStart"
             ,_0: a};
   };
   var DeviceManageModeEnd = function (a) {
      return {ctor: "DeviceManageModeEnd"
             ,_0: a};
   };
   var DeviceManageModeStart = function (a) {
      return {ctor: "DeviceManageModeStart"
             ,_0: a};
   };
   var DeviceGetRandomNumber = function (a) {
      return {ctor: "DeviceGetRandomNumber"
             ,_0: a};
   };
   var DeviceImportEepromEnd = function (a) {
      return {ctor: "DeviceImportEepromEnd"
             ,_0: a};
   };
   var DeviceImportEeprom = function (a) {
      return {ctor: "DeviceImportEeprom"
             ,_0: a};
   };
   var DeviceImportEepromStart = function (a) {
      return {ctor: "DeviceImportEepromStart"
             ,_0: a};
   };
   var DeviceExportEepromEnd = {ctor: "DeviceExportEepromEnd"};
   var DeviceExportEeprom = function (a) {
      return {ctor: "DeviceExportEeprom"
             ,_0: a};
   };
   var DeviceExportEepromStart = function (a) {
      return {ctor: "DeviceExportEepromStart"
             ,_0: a};
   };
   var DeviceImportFlashEnd = function (a) {
      return {ctor: "DeviceImportFlashEnd"
             ,_0: a};
   };
   var DeviceImportFlash = function (a) {
      return {ctor: "DeviceImportFlash"
             ,_0: a};
   };
   var DeviceImportFlashStart = function (a) {
      return {ctor: "DeviceImportFlashStart"
             ,_0: a};
   };
   var DeviceExportFlashEnd = {ctor: "DeviceExportFlashEnd"};
   var DeviceExportFlash = function (a) {
      return {ctor: "DeviceExportFlash"
             ,_0: a};
   };
   var DeviceExportFlashStart = function (a) {
      return {ctor: "DeviceExportFlashStart"
             ,_0: a};
   };
   var DeviceAddContext = function (a) {
      return {ctor: "DeviceAddContext"
             ,_0: a};
   };
   var DeviceCheckPassword = function (a) {
      return {ctor: "DeviceCheckPassword"
             ,_0: a};
   };
   var DeviceSetPassword = function (a) {
      return {ctor: "DeviceSetPassword"
             ,_0: a};
   };
   var DeviceSetLogin = function (a) {
      return {ctor: "DeviceSetLogin"
             ,_0: a};
   };
   var DeviceGetPassword = function (a) {
      return {ctor: "DeviceGetPassword"
             ,_0: a};
   };
   var DeviceGetLogin = function (a) {
      return {ctor: "DeviceGetLogin"
             ,_0: a};
   };
   var DeviceSetContext = function (a) {
      return {ctor: "DeviceSetContext"
             ,_0: a};
   };
   var DeviceGetVersion = function (a) {
      return {ctor: "DeviceGetVersion"
             ,_0: a};
   };
   var DevicePing = function (a) {
      return {ctor: "DevicePing"
             ,_0: a};
   };
   var DeviceDebug = function (a) {
      return {ctor: "DeviceDebug"
             ,_0: a};
   };
   var fromInts = function (_v45) {
      return function () {
         switch (_v45.ctor)
         {case "::":
            switch (_v45._1.ctor)
              {case "::": return function () {
                      var maybeByteStringNull = F2(function (constructor,
                      name) {
                         return _U.cmp(_v45._0,
                         0) < 1 ? $Result.Err(A2($Basics._op["++"],
                         "Zero data returned for \'",
                         A2($Basics._op["++"],
                         name,
                         "\'"))) : _U.eq(_v45._0,
                         1) && _U.eq($List.head(_v45._1._1),
                         0) ? $Result.Ok(constructor($Maybe.Nothing)) : A2($Result.map,
                         function ($) {
                            return constructor($Maybe.Just($));
                         },
                         A2($Byte.toByteString,
                         _v45._0 - 1,
                         _v45._1._1));
                      });
                      var maybeByteString = F2(function (constructor,
                      name) {
                         return _U.cmp(_v45._0,
                         0) < 1 ? $Result.Err(A2($Basics._op["++"],
                         "Zero data returned for \'",
                         A2($Basics._op["++"],
                         name,
                         "\'"))) : _U.eq(_v45._0,
                         1) && _U.eq($List.head(_v45._1._1),
                         0) ? $Result.Ok(constructor($Maybe.Nothing)) : A2($Result.map,
                         function ($) {
                            return constructor($Maybe.Just($));
                         },
                         A2($Byte.toByteString,
                         _v45._0,
                         _v45._1._1));
                      });
                      var doneOrNotDone = F2(function (constructor,
                      name) {
                         return !_U.eq(_v45._0,
                         1) ? $Result.Err(A2($Basics._op["++"],
                         "Invalid data size for \'",
                         A2($Basics._op["++"],
                         name,
                         "\'"))) : function () {
                            var _v51 = $List.head(_v45._1._1);
                            switch (_v51)
                            {case 0:
                               return $Result.Ok(constructor(NotDone));
                               case 1:
                               return $Result.Ok(constructor(Done));}
                            return $Result.Err(A2($Basics._op["++"],
                            "Invalid data for \'",
                            A2($Basics._op["++"],
                            name,
                            "\'")));
                         }();
                      });
                      return _U.cmp(_v45._0,
                      $List.length(_v45._1._1)) > 0 ? $Result.Err("Invalid size") : function () {
                         switch (_v45._1._0)
                         {case 1: return A2($Result.map,
                              DeviceDebug,
                              A2($Byte.toByteString,
                              _v45._0,
                              _v45._1._1));
                            case 2: return _U.eq(_v45._0,
                              4) ? A2($Result.map,
                              DevicePing,
                              A2($Byte.toByteString,
                              4,
                              _v45._1._1)) : $Result.Err("Invalid data size for \'ping request\'");
                            case 3: return function () {
                                 var mpVersion = function (mpv) {
                                    return $Result.map(function (s) {
                                       return _U.insert("version",
                                       s,
                                       mpv);
                                    })(A2($Byte.toByteString,
                                    _v45._0 - 3,
                                    $List.tail(_v45._1._1)));
                                 };
                                 var flashSize = $Result.map(function (b) {
                                    return {_: {}
                                           ,flashMemSize: b};
                                 })($Byte.toByte($List.head(_v45._1._1)));
                                 return A2($Result.map,
                                 DeviceGetVersion,
                                 A2($Result.andThen,
                                 flashSize,
                                 mpVersion));
                              }();
                            case 4: return !_U.eq(_v45._0,
                              1) ? $Result.Err("Invalid data size for \'set context\'") : function () {
                                 var _v53 = $List.head(_v45._1._1);
                                 switch (_v53)
                                 {case 0:
                                    return $Result.Ok(DeviceSetContext(UnknownContext));
                                    case 1:
                                    return $Result.Ok(DeviceSetContext(ContextSet));
                                    case 3:
                                    return $Result.Ok(DeviceSetContext(NoCardForContext));}
                                 return $Result.Err("Invalid data for \'set context\'");
                              }();
                            case 5:
                            return A2(maybeByteStringNull,
                              DeviceGetLogin,
                              "get login");
                            case 6:
                            return A2(maybeByteStringNull,
                              DeviceGetPassword,
                              "get password");
                            case 7: return A2(doneOrNotDone,
                              DeviceSetLogin,
                              "set login");
                            case 8: return A2(doneOrNotDone,
                              DeviceSetPassword,
                              "set password");
                            case 9: return !_U.eq(_v45._0,
                              1) ? $Result.Err("Invalid data size for \'check password\'") : function () {
                                 var _v54 = $List.head(_v45._1._1);
                                 switch (_v54)
                                 {case 0:
                                    return $Result.Ok(DeviceCheckPassword(Incorrect));
                                    case 1:
                                    return $Result.Ok(DeviceCheckPassword(Correct));
                                    case 2:
                                    return $Result.Ok(DeviceCheckPassword(RequestBlocked));}
                                 return $Result.Err("Invalid data for \'check password\'");
                              }();
                            case 10:
                            return A2(doneOrNotDone,
                              DeviceAddContext,
                              "add context");
                            case 48: return A2($Result.map,
                              DeviceExportFlash,
                              A2($Byte.toByteString,
                              _v45._0,
                              _v45._1._1));
                            case 49:
                            return $Result.Ok(DeviceExportFlashEnd);
                            case 50:
                            return A2(doneOrNotDone,
                              DeviceImportFlashStart,
                              "import flash start");
                            case 51:
                            return A2(doneOrNotDone,
                              DeviceImportFlash,
                              "import flash");
                            case 52:
                            return A2(doneOrNotDone,
                              DeviceImportFlashEnd,
                              "import flash end");
                            case 53: return A2($Result.map,
                              DeviceExportEeprom,
                              A2($Byte.toByteString,
                              _v45._0,
                              _v45._1._1));
                            case 54:
                            return $Result.Ok(DeviceExportEepromEnd);
                            case 55:
                            return A2(doneOrNotDone,
                              DeviceImportEepromStart,
                              "import eeprom start");
                            case 56:
                            return A2(doneOrNotDone,
                              DeviceImportEeprom,
                              "import eeprom");
                            case 57:
                            return A2(doneOrNotDone,
                              DeviceImportEepromEnd,
                              "import eeprom end");
                            case 64:
                            return $Result.Err("Got DeviceEraseEeprom");
                            case 65:
                            return $Result.Err("Got DeviceEraseFlash");
                            case 66:
                            return $Result.Err("Got DeviceEraseSmc");
                            case 67:
                            return $Result.Err("Got DeviceDrawBitmap");
                            case 68:
                            return $Result.Err("Got DeviceSetFont");
                            case 69:
                            return A2(doneOrNotDone,
                              DeviceExportFlashStart,
                              "export flash start");
                            case 70:
                            return A2(doneOrNotDone,
                              DeviceExportEepromStart,
                              "export eeprom start");
                            case 71:
                            return $Result.Err("Got DeviceSetBootloaderPwd");
                            case 72:
                            return $Result.Err("Got DeviceJumpToBootloader");
                            case 73:
                            return $Result.Err("Got DeviceCloneSmartcard");
                            case 74:
                            return $Result.Err("Got DeviceStackFree");
                            case 75: return A2($Result.map,
                              DeviceGetRandomNumber,
                              A2($Byte.toByteString,
                              _v45._0,
                              _v45._1._1));
                            case 80:
                            return A2(doneOrNotDone,
                              DeviceManageModeStart,
                              "start memory management mode");
                            case 81:
                            return A2(doneOrNotDone,
                              DeviceManageModeEnd,
                              "end memory management mode");
                            case 82:
                            return A2(doneOrNotDone,
                              DeviceImportMediaStart,
                              "media import start");
                            case 83:
                            return A2(doneOrNotDone,
                              DeviceImportMedia,
                              "media import");
                            case 84:
                            return A2(doneOrNotDone,
                              DeviceImportMediaEnd,
                              "media import end");
                            case 85: return A2($Result.map,
                              DeviceReadFlashNode,
                              A2($Byte.toByteString,
                              _v45._0,
                              _v45._1._1));
                            case 86:
                            return A2(doneOrNotDone,
                              DeviceWriteFlashNode,
                              "write node in flash");
                            case 87:
                            return A2(doneOrNotDone,
                              DeviceSetFavorite,
                              "set favorite");
                            case 88:
                            return A2(doneOrNotDone,
                              DeviceSetStartingParent,
                              "set starting parent");
                            case 89:
                            return A2(doneOrNotDone,
                              DeviceSetCtrValue,
                              "set CTR value");
                            case 90:
                            return A2(doneOrNotDone,
                              DeviceAddCpzCtr,
                              "set CPZ CTR value");
                            case 91:
                            return A2(maybeByteString,
                              DeviceGetCpzCtrValues,
                              "get CPZ CTR value");
                            case 92: return function () {
                                 var ctrNonce = function (d) {
                                    return $Result.map(function (s) {
                                       return _U.insert("ctrNonce",
                                       s,
                                       d);
                                    })(A2($Byte.toByteString,
                                    16,
                                    A2($List.drop,8,_v45._1._1)));
                                 };
                                 var cpz = $Result.map(function (c) {
                                    return {_: {},cpz: c};
                                 })(A2($Byte.toByteString,
                                 8,
                                 _v45._1._1));
                                 return A2($Result.map,
                                 DeviceCpzCtrPacketExport,
                                 A2($Result.andThen,
                                 cpz,
                                 ctrNonce));
                              }();
                            case 93:
                            return A2(doneOrNotDone,
                              DeviceSetParameter,
                              "set Mooltipass parameter");
                            case 94:
                            return A2(maybeByteString,
                              DeviceGetParameter,
                              "get parameter");
                            case 95:
                            return A2(maybeByteString,
                              DeviceGetFavorite,
                              "get favorite");
                            case 96:
                            return A2(doneOrNotDone,
                              DeviceResetCard,
                              "reset card");
                            case 97:
                            return A2(maybeByteStringNull,
                              DeviceGetCardLogin,
                              "get card login");
                            case 98:
                            return A2(maybeByteStringNull,
                              DeviceGetCardPassword,
                              "get card password");
                            case 99:
                            return A2(doneOrNotDone,
                              DeviceSetCardLogin,
                              "set card password");
                            case 100:
                            return A2(doneOrNotDone,
                              DeviceSetCardPassword,
                              "set card password");
                            case 101:
                            return A2(maybeByteString,
                              DeviceGetFreeSlotAddr,
                              "get free slot address");
                            case 102:
                            return A2(maybeByteString,
                              DeviceGetStartingParent,
                              "get starting parent address");
                            case 103:
                            return A2(maybeByteString,
                              DeviceGetCtrValue,
                              "get CTR value");
                            case 104:
                            return A2(doneOrNotDone,
                              DeviceAddNewCard,
                              "add unknown smartcard");
                            case 105:
                            return $Result.Err("Got DeviceUsbKeyboardPress");
                            case 112: return !_U.eq(_v45._0,
                              1) ? $Result.Err("Invalid data size for \'get status\'") : function () {
                                 var _v55 = A2($Bitwise.and,
                                 $List.head(_v45._1._1),
                                 7);
                                 switch (_v55)
                                 {case 0:
                                    return $Result.Ok(DeviceGetStatus(NeedCard));
                                    case 1:
                                    return $Result.Ok(DeviceGetStatus(Locked));
                                    case 3:
                                    return $Result.Ok(DeviceGetStatus(LockScreen));
                                    case 5:
                                    return $Result.Ok(DeviceGetStatus(Unlocked));}
                                 return $Result.Err("Invalid status received in \'get status\'");
                              }();}
                         return $Result.Err(A2($Basics._op["++"],
                         "Got unknown message: ",
                         $Basics.toString(_v45._1._0)));
                      }();
                   }();}
              break;}
         _U.badCase($moduleName,
         "between lines 253 and 371");
      }();
   };
   var AppGetStatus = {ctor: "AppGetStatus"};
   var AppAddNewCard = {ctor: "AppAddNewCard"};
   var AppGetCtrValue = {ctor: "AppGetCtrValue"};
   var AppGetStartingParent = {ctor: "AppGetStartingParent"};
   var AppGetFreeSlotAddress = {ctor: "AppGetFreeSlotAddress"};
   var AppSetCardPassword = function (a) {
      return {ctor: "AppSetCardPassword"
             ,_0: a};
   };
   var AppSetCardLogin = function (a) {
      return {ctor: "AppSetCardLogin"
             ,_0: a};
   };
   var AppGetCardPassword = {ctor: "AppGetCardPassword"};
   var AppGetCardLogin = {ctor: "AppGetCardLogin"};
   var AppResetCard = function (a) {
      return {ctor: "AppResetCard"
             ,_0: a};
   };
   var AppGetFavorite = function (a) {
      return {ctor: "AppGetFavorite"
             ,_0: a};
   };
   var AppGetParameter = function (a) {
      return {ctor: "AppGetParameter"
             ,_0: a};
   };
   var AppSetParameter = F2(function (a,
   b) {
      return {ctor: "AppSetParameter"
             ,_0: a
             ,_1: b};
   });
   var AppGetCpzCtrValues = {ctor: "AppGetCpzCtrValues"};
   var AppAddCpzCtr = function (a) {
      return {ctor: "AppAddCpzCtr"
             ,_0: a};
   };
   var AppSetCtrValue = function (a) {
      return {ctor: "AppSetCtrValue"
             ,_0: a};
   };
   var AppSetStartingParent = function (a) {
      return {ctor: "AppSetStartingParent"
             ,_0: a};
   };
   var AppSetFavorite = F4(function (a,
   b,
   c,
   d) {
      return {ctor: "AppSetFavorite"
             ,_0: a
             ,_1: b
             ,_2: c
             ,_3: d};
   });
   var AppWriteFlashNode = F3(function (a,
   b,
   c) {
      return {ctor: "AppWriteFlashNode"
             ,_0: a
             ,_1: b
             ,_2: c};
   });
   var AppReadFlashNode = function (a) {
      return {ctor: "AppReadFlashNode"
             ,_0: a};
   };
   var AppImportMediaEnd = {ctor: "AppImportMediaEnd"};
   var AppImportMedia = function (a) {
      return {ctor: "AppImportMedia"
             ,_0: a};
   };
   var AppImportMediaStart = {ctor: "AppImportMediaStart"};
   var AppMemoryManageModeEnd = {ctor: "AppMemoryManageModeEnd"};
   var AppMemoryManageModeStart = {ctor: "AppMemoryManageModeStart"};
   var AppGetRandomNumber = {ctor: "AppGetRandomNumber"};
   var AppImportEepromEnd = {ctor: "AppImportEepromEnd"};
   var AppImportEeprom = function (a) {
      return {ctor: "AppImportEeprom"
             ,_0: a};
   };
   var AppImportEepromStart = {ctor: "AppImportEepromStart"};
   var AppExportEepromEnd = {ctor: "AppExportEepromEnd"};
   var AppExportEeprom = {ctor: "AppExportEeprom"};
   var AppExportEepromStart = {ctor: "AppExportEepromStart"};
   var AppImportFlashEnd = {ctor: "AppImportFlashEnd"};
   var AppImportFlash = function (a) {
      return {ctor: "AppImportFlash"
             ,_0: a};
   };
   var AppImportFlashStart = function (a) {
      return {ctor: "AppImportFlashStart"
             ,_0: a};
   };
   var AppExportFlashEnd = {ctor: "AppExportFlashEnd"};
   var AppExportFlash = {ctor: "AppExportFlash"};
   var AppExportFlashStart = {ctor: "AppExportFlashStart"};
   var AppAddContext = function (a) {
      return {ctor: "AppAddContext"
             ,_0: a};
   };
   var AppCheckPassword = {ctor: "AppCheckPassword"};
   var AppSetPassword = function (a) {
      return {ctor: "AppSetPassword"
             ,_0: a};
   };
   var AppSetLogin = function (a) {
      return {ctor: "AppSetLogin"
             ,_0: a};
   };
   var AppGetPassword = {ctor: "AppGetPassword"};
   var AppGetLogin = {ctor: "AppGetLogin"};
   var AppSetContext = function (a) {
      return {ctor: "AppSetContext"
             ,_0: a};
   };
   var AppGetVersion = {ctor: "AppGetVersion"};
   var AppPing = {ctor: "AppPing"};
   var AppDebug = function (a) {
      return {ctor: "AppDebug"
             ,_0: a};
   };
   _elm.DevicePacket.values = {_op: _op
                              ,AppDebug: AppDebug
                              ,AppPing: AppPing
                              ,AppGetVersion: AppGetVersion
                              ,AppSetContext: AppSetContext
                              ,AppGetLogin: AppGetLogin
                              ,AppGetPassword: AppGetPassword
                              ,AppSetLogin: AppSetLogin
                              ,AppSetPassword: AppSetPassword
                              ,AppCheckPassword: AppCheckPassword
                              ,AppAddContext: AppAddContext
                              ,AppExportFlashStart: AppExportFlashStart
                              ,AppExportFlash: AppExportFlash
                              ,AppExportFlashEnd: AppExportFlashEnd
                              ,AppImportFlashStart: AppImportFlashStart
                              ,AppImportFlash: AppImportFlash
                              ,AppImportFlashEnd: AppImportFlashEnd
                              ,AppExportEepromStart: AppExportEepromStart
                              ,AppExportEeprom: AppExportEeprom
                              ,AppExportEepromEnd: AppExportEepromEnd
                              ,AppImportEepromStart: AppImportEepromStart
                              ,AppImportEeprom: AppImportEeprom
                              ,AppImportEepromEnd: AppImportEepromEnd
                              ,AppGetRandomNumber: AppGetRandomNumber
                              ,AppMemoryManageModeStart: AppMemoryManageModeStart
                              ,AppMemoryManageModeEnd: AppMemoryManageModeEnd
                              ,AppImportMediaStart: AppImportMediaStart
                              ,AppImportMedia: AppImportMedia
                              ,AppImportMediaEnd: AppImportMediaEnd
                              ,AppReadFlashNode: AppReadFlashNode
                              ,AppWriteFlashNode: AppWriteFlashNode
                              ,AppSetFavorite: AppSetFavorite
                              ,AppSetStartingParent: AppSetStartingParent
                              ,AppSetCtrValue: AppSetCtrValue
                              ,AppAddCpzCtr: AppAddCpzCtr
                              ,AppGetCpzCtrValues: AppGetCpzCtrValues
                              ,AppSetParameter: AppSetParameter
                              ,AppGetParameter: AppGetParameter
                              ,AppGetFavorite: AppGetFavorite
                              ,AppResetCard: AppResetCard
                              ,AppGetCardLogin: AppGetCardLogin
                              ,AppGetCardPassword: AppGetCardPassword
                              ,AppSetCardLogin: AppSetCardLogin
                              ,AppSetCardPassword: AppSetCardPassword
                              ,AppGetFreeSlotAddress: AppGetFreeSlotAddress
                              ,AppGetStartingParent: AppGetStartingParent
                              ,AppGetCtrValue: AppGetCtrValue
                              ,AppAddNewCard: AppAddNewCard
                              ,AppGetStatus: AppGetStatus
                              ,DeviceDebug: DeviceDebug
                              ,DevicePing: DevicePing
                              ,DeviceGetVersion: DeviceGetVersion
                              ,DeviceSetContext: DeviceSetContext
                              ,DeviceGetLogin: DeviceGetLogin
                              ,DeviceGetPassword: DeviceGetPassword
                              ,DeviceSetLogin: DeviceSetLogin
                              ,DeviceSetPassword: DeviceSetPassword
                              ,DeviceCheckPassword: DeviceCheckPassword
                              ,DeviceAddContext: DeviceAddContext
                              ,DeviceExportFlashStart: DeviceExportFlashStart
                              ,DeviceExportFlash: DeviceExportFlash
                              ,DeviceExportFlashEnd: DeviceExportFlashEnd
                              ,DeviceImportFlashStart: DeviceImportFlashStart
                              ,DeviceImportFlash: DeviceImportFlash
                              ,DeviceImportFlashEnd: DeviceImportFlashEnd
                              ,DeviceExportEepromStart: DeviceExportEepromStart
                              ,DeviceExportEeprom: DeviceExportEeprom
                              ,DeviceExportEepromEnd: DeviceExportEepromEnd
                              ,DeviceImportEepromStart: DeviceImportEepromStart
                              ,DeviceImportEeprom: DeviceImportEeprom
                              ,DeviceImportEepromEnd: DeviceImportEepromEnd
                              ,DeviceGetRandomNumber: DeviceGetRandomNumber
                              ,DeviceManageModeStart: DeviceManageModeStart
                              ,DeviceManageModeEnd: DeviceManageModeEnd
                              ,DeviceImportMediaStart: DeviceImportMediaStart
                              ,DeviceImportMediaEnd: DeviceImportMediaEnd
                              ,DeviceImportMedia: DeviceImportMedia
                              ,DeviceReadFlashNode: DeviceReadFlashNode
                              ,DeviceWriteFlashNode: DeviceWriteFlashNode
                              ,DeviceSetFavorite: DeviceSetFavorite
                              ,DeviceSetStartingParent: DeviceSetStartingParent
                              ,DeviceSetCtrValue: DeviceSetCtrValue
                              ,DeviceAddCpzCtr: DeviceAddCpzCtr
                              ,DeviceGetCpzCtrValues: DeviceGetCpzCtrValues
                              ,DeviceCpzCtrPacketExport: DeviceCpzCtrPacketExport
                              ,DeviceSetParameter: DeviceSetParameter
                              ,DeviceGetParameter: DeviceGetParameter
                              ,DeviceGetFavorite: DeviceGetFavorite
                              ,DeviceResetCard: DeviceResetCard
                              ,DeviceGetCardLogin: DeviceGetCardLogin
                              ,DeviceGetCardPassword: DeviceGetCardPassword
                              ,DeviceSetCardLogin: DeviceSetCardLogin
                              ,DeviceSetCardPassword: DeviceSetCardPassword
                              ,DeviceGetFreeSlotAddr: DeviceGetFreeSlotAddr
                              ,DeviceGetStartingParent: DeviceGetStartingParent
                              ,DeviceGetCtrValue: DeviceGetCtrValue
                              ,DeviceAddNewCard: DeviceAddNewCard
                              ,DeviceGetStatus: DeviceGetStatus
                              ,MpVersion: MpVersion
                              ,CpzCtrLutEntry: CpzCtrLutEntry
                              ,Incorrect: Incorrect
                              ,Correct: Correct
                              ,RequestBlocked: RequestBlocked
                              ,UnknownContext: UnknownContext
                              ,ContextSet: ContextSet
                              ,NoCardForContext: NoCardForContext
                              ,NeedCard: NeedCard
                              ,Locked: Locked
                              ,LockScreen: LockScreen
                              ,Unlocked: Unlocked
                              ,Done: Done
                              ,NotDone: NotDone
                              ,UserInitKey: UserInitKey
                              ,KeyboardLayout: KeyboardLayout
                              ,UserInterTimeout: UserInterTimeout
                              ,LockTimeoutEnable: LockTimeoutEnable
                              ,LockTimeout: LockTimeout
                              ,TouchDi: TouchDi
                              ,TouchWheelOs: TouchWheelOs
                              ,TouchProxOs: TouchProxOs
                              ,OfflineMode: OfflineMode
                              ,FlashUserSpace: FlashUserSpace
                              ,FlashGraphicsSpace: FlashGraphicsSpace
                              ,toInts: toInts
                              ,fromInts: fromInts};
   return _elm.DevicePacket.values;
};
Elm.ExtensionMessage = Elm.ExtensionMessage || {};
Elm.ExtensionMessage.make = function (_elm) {
   "use strict";
   _elm.ExtensionMessage = _elm.ExtensionMessage || {};
   if (_elm.ExtensionMessage.values)
   return _elm.ExtensionMessage.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "ExtensionMessage",
   $BackgroundState = Elm.BackgroundState.make(_elm),
   $Basics = Elm.Basics.make(_elm),
   $Byte = Elm.Byte.make(_elm),
   $CommonState = Elm.CommonState.make(_elm),
   $Maybe = Elm.Maybe.make(_elm),
   $Result = Elm.Result.make(_elm),
   $Util = Elm.Util.make(_elm);
   var decode = function (message) {
      return function () {
         var errOrUpdate$ = function (_v0) {
            return function () {
               return function () {
                  var bSp = $Byte.byteString(_v0.password);
                  var bSl = $Byte.byteString(_v0.login);
                  var bSc = $Byte.byteString(_v0.context);
                  return A2($Result.andThen,
                  A2($Result.andThen,
                  A2($Result.andThen,
                  bSc,
                  function (_v2) {
                     return function () {
                        return bSl;
                     }();
                  }),
                  function (_v4) {
                     return function () {
                        return bSp;
                     }();
                  }),
                  function (_v6) {
                     return function () {
                        return $Result.Ok({ctor: "_Tuple0"});
                     }();
                  });
               }();
            }();
         };
         var errOrUpdate = A2($Maybe.map,
         errOrUpdate$,
         message.update);
         var errOrInputs = A2($Maybe.map,
         function (_v8) {
            return function () {
               return $Byte.byteString(_v8.context);
            }();
         },
         message.getInputs);
         var set = F2(function (constructor,
         d) {
            return $BackgroundState.SetExtensionRequest(constructor(d));
         });
         var decode$ = function (_v10) {
            return function () {
               return $Maybe.oneOf(_L.fromArray([A2($Maybe.map,
                                                function (_v12) {
                                                   return function () {
                                                      return $BackgroundState.SetExtAwaitingPing(true);
                                                   }();
                                                },
                                                _v10.ping)
                                                ,A2($Maybe.map,
                                                set($BackgroundState.ExtWantsCredentials),
                                                _v10.getInputs)
                                                ,A2($Maybe.map,
                                                set($BackgroundState.ExtWantsToWrite),
                                                _v10.update)]));
            }();
         };
         return function () {
            switch (errOrUpdate.ctor)
            {case "Just":
               switch (errOrUpdate._0.ctor)
                 {case "Err":
                    return $BackgroundState.CommonAction($CommonState.AppendToLog(A2($Basics._op["++"],
                      "Extension Error: ",
                      errOrUpdate._0._0)));}
                 break;}
            return function () {
               switch (errOrInputs.ctor)
               {case "Just":
                  switch (errOrInputs._0.ctor)
                    {case "Err":
                       return $BackgroundState.CommonAction($CommonState.AppendToLog(A2($Basics._op["++"],
                         "Extension Error: ",
                         errOrInputs._0._0)));}
                    break;}
               return A2($Maybe.withDefault,
               $BackgroundState.NoOp,
               decode$(message));
            }();
         }();
      }();
   };
   var emptyToExtensionMessage = {_: {}
                                 ,connectState: $Maybe.Nothing
                                 ,credentials: $Maybe.Nothing
                                 ,noCredentials: $Maybe.Nothing
                                 ,updateComplete: $Maybe.Nothing};
   var encode = function (s) {
      return function () {
         var e = emptyToExtensionMessage;
         return s.extAwaitingPing && $Util.isJust(s.deviceVersion) ? {ctor: "_Tuple2"
                                                                     ,_0: _U.replace([["connectState"
                                                                                      ,A2($Maybe.map,
                                                                                      function (v) {
                                                                                         return function () {
                                                                                            var _v20 = s.common.connected;
                                                                                            switch (_v20.ctor)
                                                                                            {case "Connected":
                                                                                               return {_: {}
                                                                                                      ,state: "connected"
                                                                                                      ,version: v.version};}
                                                                                            return {_: {}
                                                                                                   ,state: "disconnected"
                                                                                                   ,version: ""};
                                                                                         }();
                                                                                      },
                                                                                      s.deviceVersion)]],
                                                                     e)
                                                                     ,_1: $BackgroundState.SetExtAwaitingPing(false)} : s.extAwaitingPing || $Basics.not(s.deviceConnected) ? {ctor: "_Tuple2"
                                                                                                                                                                              ,_0: _U.replace([["connectState"
                                                                                                                                                                                               ,$Maybe.Just({_: {}
                                                                                                                                                                                                            ,state: "disconnected"
                                                                                                                                                                                                            ,version: ""})]],
                                                                                                                                                                              e)
                                                                                                                                                                              ,_1: $BackgroundState.SetExtAwaitingPing(false)} : !_U.eq(s.extRequest,
         $BackgroundState.NoRequest) ? function () {
            var _v21 = s.extRequest;
            switch (_v21.ctor)
            {case "ExtCredentials":
               return {ctor: "_Tuple2"
                      ,_0: _U.replace([["credentials"
                                       ,$Maybe.Just(_v21._0)]],
                      e)
                      ,_1: $BackgroundState.SetExtensionRequest($BackgroundState.NoRequest)};
               case "ExtNoCredentials":
               return {ctor: "_Tuple2"
                      ,_0: _U.replace([["noCredentials"
                                       ,$Maybe.Just({ctor: "_Tuple0"})]],
                      e)
                      ,_1: $BackgroundState.SetExtensionRequest($BackgroundState.NoRequest)};
               case "ExtNotWritten":
               return {ctor: "_Tuple2"
                      ,_0: e
                      ,_1: $BackgroundState.SetExtensionRequest($BackgroundState.NoRequest)};
               case "ExtWriteComplete":
               return {ctor: "_Tuple2"
                      ,_0: _U.replace([["updateComplete"
                                       ,$Maybe.Just({ctor: "_Tuple0"})]],
                      e)
                      ,_1: $BackgroundState.SetExtensionRequest($BackgroundState.NoRequest)};}
            return {ctor: "_Tuple2"
                   ,_0: e
                   ,_1: $BackgroundState.NoOp};
         }() : {ctor: "_Tuple2"
               ,_0: e
               ,_1: $BackgroundState.NoOp};
      }();
   };
   var ToExtensionMessage = F4(function (a,
   b,
   c,
   d) {
      return {_: {}
             ,connectState: a
             ,credentials: b
             ,noCredentials: c
             ,updateComplete: d};
   });
   var FromExtensionMessage = F3(function (a,
   b,
   c) {
      return {_: {}
             ,getInputs: b
             ,ping: a
             ,update: c};
   });
   _elm.ExtensionMessage.values = {_op: _op
                                  ,FromExtensionMessage: FromExtensionMessage
                                  ,ToExtensionMessage: ToExtensionMessage
                                  ,emptyToExtensionMessage: emptyToExtensionMessage
                                  ,decode: decode
                                  ,encode: encode};
   return _elm.ExtensionMessage.values;
};
Elm.FromGuiMessage = Elm.FromGuiMessage || {};
Elm.FromGuiMessage.make = function (_elm) {
   "use strict";
   _elm.FromGuiMessage = _elm.FromGuiMessage || {};
   if (_elm.FromGuiMessage.values)
   return _elm.FromGuiMessage.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "FromGuiMessage",
   $CommonState = Elm.CommonState.make(_elm),
   $Maybe = Elm.Maybe.make(_elm);
   var decode = function (msg) {
      return function () {
         var _v0 = msg.setLog;
         switch (_v0.ctor)
         {case "Just":
            return $CommonState.SetLog(_v0._0);
            case "Nothing":
            return $CommonState.CommonNoOp;}
         _U.badCase($moduleName,
         "between lines 18 and 20");
      }();
   };
   var encode = function (action) {
      return function () {
         switch (action.ctor)
         {case "GetState": return {_: {}
                                  ,getState: $Maybe.Just({ctor: "_Tuple0"})
                                  ,setLog: $Maybe.Nothing};
            case "SetLog": return {_: {}
                                  ,getState: $Maybe.Nothing
                                  ,setLog: $Maybe.Just(action._0)};}
         return {_: {}
                ,getState: $Maybe.Nothing
                ,setLog: $Maybe.Nothing};
      }();
   };
   var FromGuiMessage = F2(function (a,
   b) {
      return {_: {}
             ,getState: b
             ,setLog: a};
   });
   _elm.FromGuiMessage.values = {_op: _op
                                ,FromGuiMessage: FromGuiMessage
                                ,encode: encode
                                ,decode: decode};
   return _elm.FromGuiMessage.values;
};
Elm.List = Elm.List || {};
Elm.List.make = function (_elm) {
   "use strict";
   _elm.List = _elm.List || {};
   if (_elm.List.values)
   return _elm.List.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "List",
   $Basics = Elm.Basics.make(_elm),
   $Maybe = Elm.Maybe.make(_elm),
   $Native$List = Elm.Native.List.make(_elm);
   var sortWith = $Native$List.sortWith;
   var sortBy = $Native$List.sortBy;
   var sort = $Native$List.sort;
   var repeat = $Native$List.repeat;
   var drop = $Native$List.drop;
   var take = $Native$List.take;
   var map5 = $Native$List.map5;
   var map4 = $Native$List.map4;
   var map3 = $Native$List.map3;
   var map2 = $Native$List.map2;
   var append = $Native$List.append;
   var any = $Native$List.any;
   var all = $Native$List.all;
   var length = $Native$List.length;
   var filter = $Native$List.filter;
   var scanl1 = $Native$List.scanl1;
   var scanl = $Native$List.scanl;
   var foldr1 = $Native$List.foldr1;
   var foldl1 = $Native$List.foldl1;
   var maximum = foldl1($Basics.max);
   var minimum = foldl1($Basics.min);
   var foldr = $Native$List.foldr;
   var concat = function (lists) {
      return A3(foldr,
      append,
      _L.fromArray([]),
      lists);
   };
   var foldl = $Native$List.foldl;
   var sum = function (numbers) {
      return A3(foldl,
      F2(function (x,y) {
         return x + y;
      }),
      0,
      numbers);
   };
   var product = function (numbers) {
      return A3(foldl,
      F2(function (x,y) {
         return x * y;
      }),
      1,
      numbers);
   };
   var indexedMap = F2(function (f,
   xs) {
      return A3(map2,
      f,
      _L.range(0,length(xs) - 1),
      xs);
   });
   var map = $Native$List.map;
   var concatMap = F2(function (f,
   list) {
      return concat(A2(map,
      f,
      list));
   });
   var member = $Native$List.member;
   var isEmpty = function (xs) {
      return function () {
         switch (xs.ctor)
         {case "[]": return true;}
         return false;
      }();
   };
   var tail = $Native$List.tail;
   var head = $Native$List.head;
   _op["::"] = $Native$List.cons;
   var maybeCons = F3(function (f,
   mx,
   xs) {
      return function () {
         var _v1 = f(mx);
         switch (_v1.ctor)
         {case "Just":
            return A2(_op["::"],_v1._0,xs);
            case "Nothing": return xs;}
         _U.badCase($moduleName,
         "between lines 162 and 169");
      }();
   });
   var filterMap = F2(function (f,
   xs) {
      return A3(foldr,
      maybeCons(f),
      _L.fromArray([]),
      xs);
   });
   var reverse = A2(foldl,
   F2(function (x,y) {
      return A2(_op["::"],x,y);
   }),
   _L.fromArray([]));
   var partition = F2(function (pred,
   list) {
      return function () {
         var step = F2(function (x,
         _v3) {
            return function () {
               switch (_v3.ctor)
               {case "_Tuple2":
                  return pred(x) ? {ctor: "_Tuple2"
                                   ,_0: A2(_op["::"],x,_v3._0)
                                   ,_1: _v3._1} : {ctor: "_Tuple2"
                                                  ,_0: _v3._0
                                                  ,_1: A2(_op["::"],x,_v3._1)};}
               _U.badCase($moduleName,
               "between lines 271 and 273");
            }();
         });
         return A3(foldr,
         step,
         {ctor: "_Tuple2"
         ,_0: _L.fromArray([])
         ,_1: _L.fromArray([])},
         list);
      }();
   });
   var unzip = function (pairs) {
      return function () {
         var step = F2(function (_v7,
         _v8) {
            return function () {
               switch (_v8.ctor)
               {case "_Tuple2":
                  return function () {
                       switch (_v7.ctor)
                       {case "_Tuple2":
                          return {ctor: "_Tuple2"
                                 ,_0: A2(_op["::"],_v7._0,_v8._0)
                                 ,_1: A2(_op["::"],
                                 _v7._1,
                                 _v8._1)};}
                       _U.badCase($moduleName,
                       "on line 309, column 12 to 28");
                    }();}
               _U.badCase($moduleName,
               "on line 309, column 12 to 28");
            }();
         });
         return A3(foldr,
         step,
         {ctor: "_Tuple2"
         ,_0: _L.fromArray([])
         ,_1: _L.fromArray([])},
         pairs);
      }();
   };
   var intersperse = F2(function (sep,
   xs) {
      return function () {
         switch (xs.ctor)
         {case "::": return function () {
                 var step = F2(function (x,
                 rest) {
                    return A2(_op["::"],
                    sep,
                    A2(_op["::"],x,rest));
                 });
                 var spersed = A3(foldr,
                 step,
                 _L.fromArray([]),
                 xs._1);
                 return A2(_op["::"],
                 xs._0,
                 spersed);
              }();
            case "[]":
            return _L.fromArray([]);}
         _U.badCase($moduleName,
         "between lines 320 and 331");
      }();
   });
   _elm.List.values = {_op: _op
                      ,head: head
                      ,tail: tail
                      ,isEmpty: isEmpty
                      ,member: member
                      ,map: map
                      ,indexedMap: indexedMap
                      ,foldl: foldl
                      ,foldr: foldr
                      ,foldl1: foldl1
                      ,foldr1: foldr1
                      ,scanl: scanl
                      ,scanl1: scanl1
                      ,filter: filter
                      ,filterMap: filterMap
                      ,maybeCons: maybeCons
                      ,length: length
                      ,reverse: reverse
                      ,all: all
                      ,any: any
                      ,append: append
                      ,concat: concat
                      ,concatMap: concatMap
                      ,sum: sum
                      ,product: product
                      ,maximum: maximum
                      ,minimum: minimum
                      ,partition: partition
                      ,map2: map2
                      ,map3: map3
                      ,map4: map4
                      ,map5: map5
                      ,unzip: unzip
                      ,intersperse: intersperse
                      ,take: take
                      ,drop: drop
                      ,repeat: repeat
                      ,sort: sort
                      ,sortBy: sortBy
                      ,sortWith: sortWith};
   return _elm.List.values;
};
Elm.Maybe = Elm.Maybe || {};
Elm.Maybe.make = function (_elm) {
   "use strict";
   _elm.Maybe = _elm.Maybe || {};
   if (_elm.Maybe.values)
   return _elm.Maybe.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Maybe";
   var withDefault = F2(function ($default,
   maybe) {
      return function () {
         switch (maybe.ctor)
         {case "Just": return maybe._0;
            case "Nothing":
            return $default;}
         _U.badCase($moduleName,
         "between lines 45 and 56");
      }();
   });
   var Nothing = {ctor: "Nothing"};
   var oneOf = function (maybes) {
      return function () {
         switch (maybes.ctor)
         {case "::": return function () {
                 switch (maybes._0.ctor)
                 {case "Just": return maybes._0;
                    case "Nothing":
                    return oneOf(maybes._1);}
                 _U.badCase($moduleName,
                 "between lines 64 and 73");
              }();
            case "[]": return Nothing;}
         _U.badCase($moduleName,
         "between lines 59 and 73");
      }();
   };
   var andThen = F2(function (maybeValue,
   callback) {
      return function () {
         switch (maybeValue.ctor)
         {case "Just":
            return callback(maybeValue._0);
            case "Nothing": return Nothing;}
         _U.badCase($moduleName,
         "between lines 110 and 112");
      }();
   });
   var Just = function (a) {
      return {ctor: "Just",_0: a};
   };
   var map = F2(function (f,
   maybe) {
      return function () {
         switch (maybe.ctor)
         {case "Just":
            return Just(f(maybe._0));
            case "Nothing": return Nothing;}
         _U.badCase($moduleName,
         "between lines 76 and 107");
      }();
   });
   _elm.Maybe.values = {_op: _op
                       ,andThen: andThen
                       ,map: map
                       ,withDefault: withDefault
                       ,oneOf: oneOf
                       ,Just: Just
                       ,Nothing: Nothing};
   return _elm.Maybe.values;
};

Elm.Native.Basics = {};
Elm.Native.Basics.make = function(elm) {
  elm.Native = elm.Native || {};
  elm.Native.Basics = elm.Native.Basics || {};
  if (elm.Native.Basics.values) return elm.Native.Basics.values;

  var Utils = Elm.Native.Utils.make(elm);

  function div(a, b) {
      return (a/b)|0;
  }
  function rem(a, b) {
      return a % b;
  }
  function mod(a, b) {
        if (b === 0) {
            throw new Error("Cannot perform mod 0. Division by zero error.");
        }
        var r = a % b;
        var m = a === 0 ? 0 : (b > 0 ? (a >= 0 ? r : r+b) : -mod(-a,-b));

        return m === b ? 0 : m;
  }
  function logBase(base, n) {
      return Math.log(n) / Math.log(base);
  }
  function negate(n) {
      return -n;
  }
  function abs(n) {
      return n < 0 ? -n : n;
  }

  function min(a, b) {
      return Utils.cmp(a,b) < 0 ? a : b;
  }
  function max(a, b) {
      return Utils.cmp(a,b) > 0 ? a : b;
  }
  function clamp(lo, hi, n) {
      return Utils.cmp(n,lo) < 0 ? lo : Utils.cmp(n,hi) > 0 ? hi : n;
  }

  function xor(a, b) {
      return a !== b;
  }
  function not(b) {
      return !b;
  }
  function isInfinite(n) {
      return n === Infinity || n === -Infinity
  }

  function truncate(n) {
      return n|0;
  }

  function degrees(d) {
      return d * Math.PI / 180;
  }
  function turns(t) {
      return 2 * Math.PI * t;
  }
  function fromPolar(point) {
      var r = point._0;
      var t = point._1;
      return Utils.Tuple2(r * Math.cos(t), r * Math.sin(t));
  }
  function toPolar(point) {
      var x = point._0;
      var y = point._1;
      return Utils.Tuple2(Math.sqrt(x * x + y * y), Math.atan2(y,x));
  }

  var basics = {
      div: F2(div),
      rem: F2(rem),
      mod: F2(mod),

      pi: Math.PI,
      e: Math.E,
      cos: Math.cos,
      sin: Math.sin,
      tan: Math.tan,
      acos: Math.acos,
      asin: Math.asin,
      atan: Math.atan,
      atan2: F2(Math.atan2),

      degrees:  degrees,
      turns:  turns,
      fromPolar:  fromPolar,
      toPolar:  toPolar,

      sqrt: Math.sqrt,
      logBase: F2(logBase),
      negate: negate,
      abs: abs,
      min: F2(min),
      max: F2(max),
      clamp: F3(clamp),
      compare: Utils.compare,

      xor: F2(xor),
      not: not,

      truncate: truncate,
      ceiling: Math.ceil,
      floor: Math.floor,
      round: Math.round,
      toFloat: function(x) { return x; },
      isNaN: isNaN,
      isInfinite: isInfinite
  };

  return elm.Native.Basics.values = basics;
};

Elm.Native.Bitwise = {};
Elm.Native.Bitwise.make = function(elm) {
    elm.Native = elm.Native || {};
    elm.Native.Bitwise = elm.Native.Bitwise || {};
    if (elm.Native.Bitwise.values) return elm.Native.Bitwise.values;

    function and(a,b) { return a & b; }
    function or (a,b) { return a | b; }
    function xor(a,b) { return a ^ b; }
    function not(a) { return ~a; }
    function sll(a,offset) { return a << offset; }
    function sra(a,offset) { return a >> offset; }
    function srl(a,offset) { return a >>> offset; }

    return elm.Native.Bitwise.values = {
        and: F2(and),
        or : F2(or ),
        xor: F2(xor),
        complement: not,
        shiftLeft           : F2(sll),
        shiftRightArithmatic: F2(sra),
        shiftRightLogical   : F2(srl)
    };
    
};

Elm.Native.Char = {};
Elm.Native.Char.make = function(elm) {
    elm.Native = elm.Native || {};
    elm.Native.Char = elm.Native.Char || {};
    if (elm.Native.Char.values) return elm.Native.Char.values;

    var Utils = Elm.Native.Utils.make(elm);

    function isBetween(lo,hi) { return function(chr) {
	var c = chr.charCodeAt(0);
	return lo <= c && c <= hi;
    };
                              }
    var isDigit = isBetween('0'.charCodeAt(0),'9'.charCodeAt(0));
    var chk1 = isBetween('a'.charCodeAt(0),'f'.charCodeAt(0));
    var chk2 = isBetween('A'.charCodeAt(0),'F'.charCodeAt(0));

    return elm.Native.Char.values = {
        fromCode : function(c) { return String.fromCharCode(c); },
        toCode   : function(c) { return c.charCodeAt(0); },
        toUpper  : function(c) { return Utils.chr(c.toUpperCase()); },
        toLower  : function(c) { return Utils.chr(c.toLowerCase()); },
        toLocaleUpper : function(c) { return Utils.chr(c.toLocaleUpperCase()); },
        toLocaleLower : function(c) { return Utils.chr(c.toLocaleLowerCase()); },
        isLower    : isBetween('a'.charCodeAt(0),'z'.charCodeAt(0)),
        isUpper    : isBetween('A'.charCodeAt(0),'Z'.charCodeAt(0)),
        isDigit    : isDigit,
        isOctDigit : isBetween('0'.charCodeAt(0),'7'.charCodeAt(0)),
        isHexDigit : function(c) { return isDigit(c) || chk1(c) || chk2(c); }
    };
};

Elm.Native.List = {};
Elm.Native.List.make = function(elm) {
    elm.Native = elm.Native || {};
    elm.Native.List = elm.Native.List || {};
    if (elm.Native.List.values) return elm.Native.List.values;
    if ('values' in Elm.Native.List)
        return elm.Native.List.values = Elm.Native.List.values;

    var Utils = Elm.Native.Utils.make(elm);

    var Nil = Utils.Nil;
    var Cons = Utils.Cons;

    function throwError(f) {
        throw new Error("Function '" + f + "' expects a non-empty list!");
    }

    function toArray(xs) {
        var out = [];
        while (xs.ctor !== '[]') {
            out.push(xs._0);
            xs = xs._1;
        }
        return out;
    }

    function fromArray(arr) {
        var out = Nil;
        for (var i = arr.length; i--; ) {
            out = Cons(arr[i], out);
        }
        return out;
    }

    function range(lo,hi) {
        var lst = Nil;
        if (lo <= hi) {
            do { lst = Cons(hi,lst) } while (hi-->lo);
        }
        return lst
    }

    function head(v) {
        return v.ctor === '[]' ? throwError('head') : v._0;
    }
    function tail(v) {
        return v.ctor === '[]' ? throwError('tail') : v._1;
    }

    function map(f, xs) {
        var arr = [];
        while (xs.ctor !== '[]') {
            arr.push(f(xs._0));
            xs = xs._1;
        }
        return fromArray(arr);
    }

    // f defined similarly for both foldl and foldr (NB: different from Haskell)
    // ie, foldl : (a -> b -> b) -> b -> [a] -> b
    function foldl(f, b, xs) {
        var acc = b;
        while (xs.ctor !== '[]') {
            acc = A2(f, xs._0, acc);
            xs = xs._1;
        }
        return acc;
    }

    function foldr(f, b, xs) {
        var arr = toArray(xs);
        var acc = b;
        for (var i = arr.length; i--; ) {
            acc = A2(f, arr[i], acc);
        }
        return acc;
    }

    function foldl1(f, xs) {
        return xs.ctor === '[]' ? throwError('foldl1') : foldl(f, xs._0, xs._1);
    }

    function foldr1(f, xs) {
        if (xs.ctor === '[]') { throwError('foldr1'); }
        var arr = toArray(xs);
        var acc = arr.pop();
        for (var i = arr.length; i--; ) {
            acc = A2(f, arr[i], acc);
        }
        return acc;
    }

    function scanl(f, b, xs) {
        var arr = toArray(xs);
        arr.unshift(b);
        var len = arr.length;
        for (var i = 1; i < len; ++i) {
            arr[i] = A2(f, arr[i], arr[i-1]);
        }
        return fromArray(arr);
    }

    function scanl1(f, xs) {
        return xs.ctor === '[]' ? throwError('scanl1') : scanl(f, xs._0, xs._1);
    }

    function filter(pred, xs) {
        var arr = [];
        while (xs.ctor !== '[]') {
            if (pred(xs._0)) { arr.push(xs._0); }
            xs = xs._1;
        }
        return fromArray(arr);
    }

    function length(xs) {
        var out = 0;
        while (xs.ctor !== '[]') {
            out += 1;
            xs = xs._1;
        }
        return out;
    }

    function member(x, xs) {
        while (xs.ctor !== '[]') {
            if (Utils.eq(x,xs._0)) return true;
            xs = xs._1;
        }
        return false;
    }

    function append(xs, ys) {
        if (xs.ctor === '[]') {
            return ys;
        }
        var root = Cons(xs._0, Nil);
        var curr = root;
        xs = xs._1;
        while (xs.ctor !== '[]') {
            curr._1 = Cons(xs._0, Nil);
            xs = xs._1;
            curr = curr._1;
        }
        curr._1 = ys;
        return root;
    }

    function all(pred, xs) {
        while (xs.ctor !== '[]') {
            if (!pred(xs._0)) return false;
            xs = xs._1;
        }
        return true;
    }

    function any(pred, xs) {
        while (xs.ctor !== '[]') {
            if (pred(xs._0)) return true;
            xs = xs._1;
        }
        return false;
    }

    function map2(f, xs, ys) {
        var arr = [];
        while (xs.ctor !== '[]' && ys.ctor !== '[]') {
            arr.push(A2(f, xs._0, ys._0));
            xs = xs._1;
            ys = ys._1;
        }
        return fromArray(arr);
    }

    function map3(f, xs, ys, zs) {
        var arr = [];
        while (xs.ctor !== '[]' && ys.ctor !== '[]' && zs.ctor !== '[]') {
            arr.push(A3(f, xs._0, ys._0, zs._0));
            xs = xs._1;
            ys = ys._1;
            zs = zs._1;
        }
        return fromArray(arr);
    }

    function map4(f, ws, xs, ys, zs) {
        var arr = [];
        while (   ws.ctor !== '[]'
               && xs.ctor !== '[]'
               && ys.ctor !== '[]'
               && zs.ctor !== '[]')
        {
            arr.push(A4(f, ws._0, xs._0, ys._0, zs._0));
            ws = ws._1;
            xs = xs._1;
            ys = ys._1;
            zs = zs._1;
        }
        return fromArray(arr);
    }

    function map5(f, vs, ws, xs, ys, zs) {
        var arr = [];
        while (   vs.ctor !== '[]'
               && ws.ctor !== '[]'
               && xs.ctor !== '[]'
               && ys.ctor !== '[]'
               && zs.ctor !== '[]')
        {
            arr.push(A5(f, vs._0, ws._0, xs._0, ys._0, zs._0));
            vs = vs._1;
            ws = ws._1;
            xs = xs._1;
            ys = ys._1;
            zs = zs._1;
        }
        return fromArray(arr);
    }

    function sort(xs) {
        return fromArray(toArray(xs).sort(Utils.cmp));
    }

    function sortBy(f, xs) {
        return fromArray(toArray(xs).sort(function(a,b){
            return Utils.cmp(f(a), f(b));
        }));
    }

    function sortWith(f, xs) {
        return fromArray(toArray(xs).sort(function(a,b){
            var ord = f(a)(b).ctor;
            return ord === 'EQ' ? 0 : ord === 'LT' ? -1 : 1;
        }));
    }

    function take(n, xs) {
        var arr = [];
        while (xs.ctor !== '[]' && n > 0) {
            arr.push(xs._0);
            xs = xs._1;
            --n;
        }
        return fromArray(arr);
    }

    function drop(n, xs) {
        while (xs.ctor !== '[]' && n > 0) {
            xs = xs._1;
            --n;
        }
        return xs;
    }

    function repeat(n, x) {
        var arr = [];
        var pattern = [x];
        while (n > 0) {
            if (n & 1) arr = arr.concat(pattern);
            n >>= 1, pattern = pattern.concat(pattern);
        }
        return fromArray(arr);
    }


    Elm.Native.List.values = {
        Nil:Nil,
        Cons:Cons,
        cons:F2(Cons),
        toArray:toArray,
        fromArray:fromArray,
        range:range,
        append: F2(append),

        head:head,
        tail:tail,

        map:F2(map),
        foldl:F3(foldl),
        foldr:F3(foldr),

        foldl1:F2(foldl1),
        foldr1:F2(foldr1),
        scanl:F3(scanl),
        scanl1:F2(scanl1),
        filter:F2(filter),
        length:length,
        member:F2(member),

        all:F2(all),
        any:F2(any),
        map2:F3(map2),
        map3:F4(map3),
        map4:F5(map4),
        map5:F6(map5),
        sort:sort,
        sortBy:F2(sortBy),
        sortWith:F2(sortWith),
        take:F2(take),
        drop:F2(drop),
        repeat:F2(repeat)
    };
    return elm.Native.List.values = Elm.Native.List.values;

};

Elm.Native.Ports = {};
Elm.Native.Ports.make = function(localRuntime) {
    localRuntime.Native = localRuntime.Native || {};
    localRuntime.Native.Ports = localRuntime.Native.Ports || {};
    if (localRuntime.Native.Ports.values) {
        return localRuntime.Native.Ports.values;
    }

    var Signal;

    function incomingSignal(converter) {
        converter.isSignal = true;
        return converter;
    }

    function outgoingSignal(converter) {
        if (!Signal) {
            Signal = Elm.Signal.make(localRuntime);
        }
        return function(signal) {
            var subscribers = []
            function subscribe(handler) {
                subscribers.push(handler);
            }
            function unsubscribe(handler) {
                subscribers.pop(subscribers.indexOf(handler));
            }
            A2( Signal.map, function(value) {
                var val = converter(value);
                var len = subscribers.length;
                for (var i = 0; i < len; ++i) {
                    subscribers[i](val);
                }
            }, signal);
            return { subscribe:subscribe, unsubscribe:unsubscribe };
        }
    }

    function portIn(name, converter) {
        var jsValue = localRuntime.ports.incoming[name];
        if (jsValue === undefined) {
            throw new Error("Initialization Error: port '" + name +
                            "' was not given an input!");
        }
        localRuntime.ports.uses[name] += 1;
        try {
            var elmValue = converter(jsValue);
        } catch(e) {
            throw new Error("Initialization Error on port '" + name + "': \n" + e.message);
        }

        // just return a static value if it is not a signal
        if (!converter.isSignal) {
            return elmValue;
        }

        // create a signal if necessary
        if (!Signal) {
            Signal = Elm.Signal.make(localRuntime);
        }
        var signal = Signal.constant(elmValue);
        function send(jsValue) {
            try {
                var elmValue = converter(jsValue);
            } catch(e) {
                throw new Error("Error sending to port '" + name + "': \n" + e.message);
            }
            setTimeout(function() {
                localRuntime.notify(signal.id, elmValue);
            }, 0);
        }
        localRuntime.ports.outgoing[name] = { send:send };
        return signal;
    }

    function portOut(name, converter, value) {
        try {
            localRuntime.ports.outgoing[name] = converter(value);
        } catch(e) {
            throw new Error("Initialization Error on port '" + name + "': \n" + e.message);
        }
        return value;
    }

    return localRuntime.Native.Ports.values = {
        incomingSignal: incomingSignal,
        outgoingSignal: outgoingSignal,
        portOut: portOut,
        portIn: portIn
    };
};



if (!Elm.fullscreen) {

    (function() {
        'use strict';

        var Display = { FULLSCREEN: 0, COMPONENT: 1, NONE: 2 };

        Elm.fullscreen = function(module, ports) {
            var container = document.createElement('div');
            document.body.appendChild(container);
            return init(Display.FULLSCREEN, container, module, ports || {});
        };

        Elm.embed = function(module, container, ports) {
            var tag = container.tagName;
            if (tag !== 'DIV') {
                throw new Error('Elm.node must be given a DIV, not a ' + tag + '.');
            }
            return init(Display.COMPONENT, container, module, ports || {});
        };

        Elm.worker = function(module, ports) {
            return init(Display.NONE, {}, module, ports || {});
        };

        function init(display, container, module, ports, moduleToReplace) {
            // defining state needed for an instance of the Elm RTS
            var inputs = [];

            /* OFFSET
             * Elm's time traveling debugger lets you pause time. This means
             * "now" may be shifted a bit into the past. By wrapping Date.now()
             * we can manage this.
             */
            var timer = {
                now: function() {
                    return Date.now();
                }
            };

            var updateInProgress = false;
            function notify(id, v) {
                if (updateInProgress) {
                    throw new Error(
                        'The notify function has been called synchronously!\n' +
                        'This can lead to frames being dropped.\n' +
                        'Definitely report this to <https://github.com/elm-lang/Elm/issues>\n');
                }
                updateInProgress = true;
                var timestep = timer.now();
                for (var i = inputs.length; i--; ) {
                    inputs[i].recv(timestep, id, v);
                }
                updateInProgress = false;
            }
            function setTimeout(func, delay) {
                window.setTimeout(func, delay);
            }

            var listeners = [];
            function addListener(relevantInputs, domNode, eventName, func) {
                domNode.addEventListener(eventName, func);
                var listener = {
                    relevantInputs: relevantInputs,
                    domNode: domNode,
                    eventName: eventName,
                    func: func
                };
                listeners.push(listener);
            }

            var portUses = {}
            for (var key in ports) {
                portUses[key] = 0;
            }
            // create the actual RTS. Any impure modules will attach themselves to this
            // object. This permits many Elm programs to be embedded per document.
            var elm = {
                notify: notify,
                setTimeout: setTimeout,
                node: container,
                addListener: addListener,
                inputs: inputs,
                timer: timer,
                ports: { incoming:ports, outgoing:{}, uses:portUses },

                isFullscreen: function() { return display === Display.FULLSCREEN; },
                isEmbed: function() { return display === Display.COMPONENT; },
                isWorker: function() { return display === Display.NONE; }
            };

            function swap(newModule) {
                removeListeners(listeners);
                var div = document.createElement('div');
                var newElm = init(display, div, newModule, ports, elm);
                inputs = [];
                // elm.swap = newElm.swap;
                return newElm;
            }

            function dispose() {
                removeListeners(listeners);
                inputs = [];
            }

            var Module = {};
            try {
                Module = module.make(elm);
                checkPorts(elm);
            }
            catch (error) {
                if (typeof container.appendChild == 'undefined') {
                    console.log(error.message);
                } else {
                    container.appendChild(errorNode(error.message));
                }
                throw error;
            }
            inputs = filterDeadInputs(inputs);
            filterListeners(inputs, listeners);
            addReceivers(elm.ports.outgoing);
            if (display !== Display.NONE) {
                var graphicsNode = initGraphics(elm, Module);
            }
            if (typeof moduleToReplace !== 'undefined') {
                hotSwap(moduleToReplace, elm);

                // rerender scene if graphics are enabled.
                if (typeof graphicsNode !== 'undefined') {
                    graphicsNode.recv(0, true, 0);
                }
            }

            return {
                swap:swap,
                ports:elm.ports.outgoing,
                dispose:dispose
            };
        };

        function checkPorts(elm) {
            var portUses = elm.ports.uses;
            for (var key in portUses) {
                var uses = portUses[key]
                if (uses === 0) {
                    throw new Error(
                        "Initialization Error: provided port '" + key +
                        "' to a module that does not take it as in input.\n" +
                        "Remove '" + key + "' from the module initialization code.");
                } else if (uses > 1) {
                    throw new Error(
                        "Initialization Error: port '" + key +
                        "' has been declared multiple times in the Elm code.\n" +
                        "Remove declarations until there is exactly one.");
                }
            }
        }

        function errorNode(message) {
            var code = document.createElement('code');

            var lines = message.split('\n');
            code.appendChild(document.createTextNode(lines[0]));
            code.appendChild(document.createElement('br'));
            code.appendChild(document.createElement('br'));
            for (var i = 1; i < lines.length; ++i) {
                code.appendChild(document.createTextNode('\u00A0 \u00A0 ' + lines[i]));
                code.appendChild(document.createElement('br'));
            }
            code.appendChild(document.createElement('br'));
            code.appendChild(document.createTextNode("Open the developer console for more details."));
            return code;
        }


        //// FILTER SIGNALS ////

        // TODO: move this code into the signal module and create a function
        // Signal.initializeGraph that actually instantiates everything.

        function filterListeners(inputs, listeners) {
            loop:
            for (var i = listeners.length; i--; ) {
                var listener = listeners[i];
                for (var j = inputs.length; j--; ) {
                    if (listener.relevantInputs.indexOf(inputs[j].id) >= 0) {
                        continue loop;
                    }
                }
                listener.domNode.removeEventListener(listener.eventName, listener.func);
            }
        }

        function removeListeners(listeners) {
            for (var i = listeners.length; i--; ) {
                var listener = listeners[i];
                listener.domNode.removeEventListener(listener.eventName, listener.func);
            }
        }

        // add receivers for built-in ports if they are defined
        function addReceivers(ports) {
            if ('log' in ports) {
                ports.log.subscribe(function(v) { console.log(v) });
            }
            if ('stdout' in ports) {
                var process = process || {};
                var handler = process.stdout
                    ? function(v) { process.stdout.write(v); }
                    : function(v) { console.log(v); };
                ports.stdout.subscribe(handler);
            }
            if ('stderr' in ports) {
                var process = process || {};
                var handler = process.stderr
                    ? function(v) { process.stderr.write(v); }
                    : function(v) { console.log('Error:' + v); };
                ports.stderr.subscribe(handler);
            }
            if ('title' in ports) {
                if (typeof ports.title === 'string') {
                    document.title = ports.title;
                } else {
                    ports.title.subscribe(function(v) { document.title = v; });
                }
            }
            if ('redirect' in ports) {
                ports.redirect.subscribe(function(v) {
                    if (v.length > 0) window.location = v;
                });
            }
            if ('favicon' in ports) {
                if (typeof ports.favicon === 'string') {
                    changeFavicon(ports.favicon);
                } else {
                    ports.favicon.subscribe(changeFavicon);
                }
            }
            function changeFavicon(src) {
                var link = document.createElement('link');
                var oldLink = document.getElementById('elm-favicon');
                link.id = 'elm-favicon';
                link.rel = 'shortcut icon';
                link.href = src;
                if (oldLink) {
                    document.head.removeChild(oldLink);
                }
                document.head.appendChild(link);
            }
        }


        function filterDeadInputs(inputs) {
            var temp = [];
            for (var i = inputs.length; i--; ) {
                if (isAlive(inputs[i])) temp.push(inputs[i]);
            }
            return temp;
        }


        function isAlive(input) {
            if (!('defaultNumberOfKids' in input)) return true;
            var len = input.kids.length;
            if (len === 0) return false;
            if (len > input.defaultNumberOfKids) return true;
            var alive = false;
            for (var i = len; i--; ) {
                alive = alive || isAlive(input.kids[i]);
            }
            return alive;
        }


        ////  RENDERING  ////

        function initGraphics(elm, Module) {
            if (!('main' in Module)) {
                throw new Error("'main' is missing! What do I display?!");
            }

            var signalGraph = Module.main;

            // make sure the signal graph is actually a signal & extract the visual model
            var Signal = Elm.Signal.make(elm);
            if (!('recv' in signalGraph)) {
                signalGraph = Signal.constant(signalGraph);
            }
            var initialScene = signalGraph.value;

            // Figure out what the render functions should be
            var render;
            var update;
            if (initialScene.props) {
                var Element = Elm.Native.Graphics.Element.make(elm);
                render = Element.render;
                update = Element.updateAndReplace;
            } else {
                var VirtualDom = Elm.Native.VirtualDom.make(elm);
                render = VirtualDom.render;
                update = VirtualDom.updateAndReplace;
            }

            // Add the initialScene to the DOM
            var container = elm.node;
            var node = render(initialScene);
            while (container.firstChild) {
                container.removeChild(container.firstChild);
            }
            container.appendChild(node);

            var _requestAnimationFrame =
                typeof requestAnimationFrame !== 'undefined'
                    ? requestAnimationFrame
                    : function(cb) { setTimeout(cb, 1000/60); }
                    ;

            // domUpdate is called whenever the main Signal changes.
            //
            // domUpdate and drawCallback implement a small state machine in order
            // to schedule only 1 draw per animation frame. This enforces that
            // once draw has been called, it will not be called again until the
            // next frame.
            //
            // drawCallback is scheduled whenever
            // 1. The state transitions from PENDING_REQUEST to EXTRA_REQUEST, or
            // 2. The state transitions from NO_REQUEST to PENDING_REQUEST
            //
            // Invariants:
            // 1. In the NO_REQUEST state, there is never a scheduled drawCallback.
            // 2. In the PENDING_REQUEST and EXTRA_REQUEST states, there is always exactly 1
            //    scheduled drawCallback.
            var NO_REQUEST = 0;
            var PENDING_REQUEST = 1;
            var EXTRA_REQUEST = 2;
            var state = NO_REQUEST;
            var savedScene = initialScene;
            var scheduledScene = initialScene;

            function domUpdate(newScene) {
                scheduledScene = newScene;

                switch (state) {
                    case NO_REQUEST:
                        _requestAnimationFrame(drawCallback);
                        state = PENDING_REQUEST;
                        return;
                    case PENDING_REQUEST:
                        state = PENDING_REQUEST;
                        return;
                    case EXTRA_REQUEST:
                        state = PENDING_REQUEST;
                        return;
                }
            }

            function drawCallback() {
                switch (state) {
                    case NO_REQUEST:
                        // This state should not be possible. How can there be no
                        // request, yet somehow we are actively fulfilling a
                        // request?
                        throw new Error(
                            "Unexpected draw callback.\n" +
                            "Please report this to <https://github.com/elm-lang/core/issues>."
                        );

                    case PENDING_REQUEST:
                        // At this point, we do not *know* that another frame is
                        // needed, but we make an extra request to rAF just in
                        // case. It's possible to drop a frame if rAF is called
                        // too late, so we just do it preemptively.
                        _requestAnimationFrame(drawCallback);
                        state = EXTRA_REQUEST;

                        // There's also stuff we definitely need to draw.
                        draw();
                        return;

                    case EXTRA_REQUEST:
                        // Turns out the extra request was not needed, so we will
                        // stop calling rAF. No reason to call it all the time if
                        // no one needs it.
                        state = NO_REQUEST;
                        return;
                }
            }

            function draw() {
                update(elm.node.firstChild, savedScene, scheduledScene);
                if (elm.Native.Window) {
                    elm.Native.Window.values.resizeIfNeeded();
                }
                savedScene = scheduledScene;
            }

            var renderer = A2(Signal.map, domUpdate, signalGraph);

            // must check for resize after 'renderer' is created so
            // that changes show up.
            if (elm.Native.Window) {
                elm.Native.Window.values.resizeIfNeeded();
            }

            return renderer;
        }

        //// HOT SWAPPING ////

        // Returns boolean indicating if the swap was successful.
        // Requires that the two signal graphs have exactly the same
        // structure.
        function hotSwap(from, to) {
            function similar(nodeOld,nodeNew) {
                var idOkay = nodeOld.id === nodeNew.id;
                var lengthOkay = nodeOld.kids.length === nodeNew.kids.length;
                return idOkay && lengthOkay;
            }
            function swap(nodeOld,nodeNew) {
                nodeNew.value = nodeOld.value;
                return true;
            }
            var canSwap = depthFirstTraversals(similar, from.inputs, to.inputs);
            if (canSwap) {
                depthFirstTraversals(swap, from.inputs, to.inputs);
            }
            from.node.parentNode.replaceChild(to.node, from.node);

            return canSwap;
        }

        // Returns false if the node operation f ever fails.
        function depthFirstTraversals(f, queueOld, queueNew) {
            if (queueOld.length !== queueNew.length) return false;
            queueOld = queueOld.slice(0);
            queueNew = queueNew.slice(0);

            var seen = [];
            while (queueOld.length > 0 && queueNew.length > 0) {
                var nodeOld = queueOld.pop();
                var nodeNew = queueNew.pop();
                if (seen.indexOf(nodeOld.id) < 0) {
                    if (!f(nodeOld, nodeNew)) return false;
                    queueOld = queueOld.concat(nodeOld.kids);
                    queueNew = queueNew.concat(nodeNew.kids);
                    seen.push(nodeOld.id);
                }
            }
            return true;
        }
    }());

    function F2(fun) {
        function wrapper(a) { return function(b) { return fun(a,b) } }
        wrapper.arity = 2;
        wrapper.func = fun;
        return wrapper;
    }

    function F3(fun) {
        function wrapper(a) {
            return function(b) { return function(c) { return fun(a,b,c) }}
        }
        wrapper.arity = 3;
        wrapper.func = fun;
        return wrapper;
    }

    function F4(fun) {
        function wrapper(a) { return function(b) { return function(c) {
            return function(d) { return fun(a,b,c,d) }}}
        }
        wrapper.arity = 4;
        wrapper.func = fun;
        return wrapper;
    }

    function F5(fun) {
        function wrapper(a) { return function(b) { return function(c) {
            return function(d) { return function(e) { return fun(a,b,c,d,e) }}}}
        }
        wrapper.arity = 5;
        wrapper.func = fun;
        return wrapper;
    }

    function F6(fun) {
        function wrapper(a) { return function(b) { return function(c) {
            return function(d) { return function(e) { return function(f) {
            return fun(a,b,c,d,e,f) }}}}}
        }
        wrapper.arity = 6;
        wrapper.func = fun;
        return wrapper;
    }

    function F7(fun) {
        function wrapper(a) { return function(b) { return function(c) {
            return function(d) { return function(e) { return function(f) {
            return function(g) { return fun(a,b,c,d,e,f,g) }}}}}}
        }
        wrapper.arity = 7;
        wrapper.func = fun;
        return wrapper;
    }

    function F8(fun) {
        function wrapper(a) { return function(b) { return function(c) {
            return function(d) { return function(e) { return function(f) {
            return function(g) { return function(h) {
            return fun(a,b,c,d,e,f,g,h)}}}}}}}
        }
        wrapper.arity = 8;
        wrapper.func = fun;
        return wrapper;
    }

    function F9(fun) {
        function wrapper(a) { return function(b) { return function(c) {
            return function(d) { return function(e) { return function(f) {
            return function(g) { return function(h) { return function(i) {
            return fun(a,b,c,d,e,f,g,h,i) }}}}}}}}
        }
        wrapper.arity = 9;
        wrapper.func = fun;
        return wrapper;
    }

    function A2(fun,a,b) {
        return fun.arity === 2
            ? fun.func(a,b)
            : fun(a)(b);
    }
    function A3(fun,a,b,c) {
        return fun.arity === 3
            ? fun.func(a,b,c)
            : fun(a)(b)(c);
    }
    function A4(fun,a,b,c,d) {
        return fun.arity === 4
            ? fun.func(a,b,c,d)
            : fun(a)(b)(c)(d);
    }
    function A5(fun,a,b,c,d,e) {
        return fun.arity === 5
            ? fun.func(a,b,c,d,e)
            : fun(a)(b)(c)(d)(e);
    }
    function A6(fun,a,b,c,d,e,f) {
        return fun.arity === 6
            ? fun.func(a,b,c,d,e,f)
            : fun(a)(b)(c)(d)(e)(f);
    }
    function A7(fun,a,b,c,d,e,f,g) {
        return fun.arity === 7
            ? fun.func(a,b,c,d,e,f,g)
            : fun(a)(b)(c)(d)(e)(f)(g);
    }
    function A8(fun,a,b,c,d,e,f,g,h) {
        return fun.arity === 8
            ? fun.func(a,b,c,d,e,f,g,h)
            : fun(a)(b)(c)(d)(e)(f)(g)(h);
    }
    function A9(fun,a,b,c,d,e,f,g,h,i) {
        return fun.arity === 9
            ? fun.func(a,b,c,d,e,f,g,h,i)
            : fun(a)(b)(c)(d)(e)(f)(g)(h)(i);
    }
}
Elm.Native.Show = {};
Elm.Native.Show.make = function(elm) {
    elm.Native = elm.Native || {};
    elm.Native.Show = elm.Native.Show || {};
    if (elm.Native.Show.values)
    {
        return elm.Native.Show.values;
    }

    var _Array;
    var Dict;
    var List;
    var Utils = Elm.Native.Utils.make(elm);

    var toString = function(v) {
        var type = typeof v;
        if (type === "function") {
            var name = v.func ? v.func.name : v.name;
            return '<function' + (name === '' ? '' : ': ') + name + '>';
        }
        else if (type === "boolean") {
            return v ? "True" : "False";
        }
        else if (type === "number") {
            return v + "";
        }
        else if ((v instanceof String) && v.isChar) {
            return "'" + addSlashes(v, true) + "'";
        }
        else if (type === "string") {
            return '"' + addSlashes(v, false) + '"';
        }
        else if (type === "object" && '_' in v && probablyPublic(v)) {
            var output = [];
            for (var k in v._) {
                for (var i = v._[k].length; i--; ) {
                    output.push(k + " = " + toString(v._[k][i]));
                }
            }
            for (var k in v) {
                if (k === '_') continue;
                output.push(k + " = " + toString(v[k]));
            }
            if (output.length === 0) {
                return "{}";
            }
            return "{ " + output.join(", ") + " }";
        }
        else if (type === "object" && 'ctor' in v) {
            if (v.ctor.substring(0,6) === "_Tuple") {
                var output = [];
                for (var k in v) {
                    if (k === 'ctor') continue;
                    output.push(toString(v[k]));
                }
                return "(" + output.join(",") + ")";
            }
            else if (v.ctor === "_Array") {
                if (!_Array) {
                    _Array = Elm.Array.make(elm);
                }
                var list = _Array.toList(v);
                return "Array.fromList " + toString(list);
            }
            else if (v.ctor === "::") {
                var output = '[' + toString(v._0);
                v = v._1;
                while (v.ctor === "::") {
                    output += "," + toString(v._0);
                    v = v._1;
                }
                return output + ']';
            }
            else if (v.ctor === "[]") {
                return "[]";
            }
            else if (v.ctor === "RBNode" || v.ctor === "RBEmpty") {
                if (!Dict) {
                    Dict = Elm.Dict.make(elm);
                }
                if (!List) {
                    List = Elm.List.make(elm);
                }
                var list = Dict.toList(v);
                var name = "Dict";
                if (list.ctor === "::" && list._0._1.ctor === "_Tuple0") {
                    name = "Set";
                    list = A2(List.map, function(x){return x._0}, list);
                }
                return name + ".fromList " + toString(list);
            }
            else {
                var output = "";
                for (var i in v) {
                    if (i === 'ctor') continue;
                    var str = toString(v[i]);
                    var parenless = str[0] === '{' || str[0] === '<' || str.indexOf(' ') < 0;
                    output += ' ' + (parenless ? str : '(' + str + ')');
                }
                return v.ctor + output;
            }
        }
        if (type === 'object' && 'recv' in v) {
            return '<signal>';
        }
        return "<internal structure>";
    };

    function addSlashes(str, isChar) {
        var s = str.replace(/\\/g, '\\\\')
                  .replace(/\n/g, '\\n')
                  .replace(/\t/g, '\\t')
                  .replace(/\r/g, '\\r')
                  .replace(/\v/g, '\\v')
                  .replace(/\0/g, '\\0');
        if (isChar) {
            return s.replace(/\'/g, "\\'")
        } else {
            return s.replace(/\"/g, '\\"');
        }
    }

    function probablyPublic(v) {
        var keys = Object.keys(v);
        var len = keys.length;
        if (len === 3
            && 'props' in v
            && 'element' in v)
        {
            return false;
        }
        else if (len === 5
            && 'horizontal' in v
            && 'vertical' in v
            && 'x' in v
            && 'y' in v)
        {
            return false;
        }
        else if (len === 7
            && 'theta' in v
            && 'scale' in v
            && 'x' in v
            && 'y' in v
            && 'alpha' in v
            && 'form' in v)
        {
            return false;
        }
        return true;
    }

    return elm.Native.Show.values = {
        toString: toString
    };
};


Elm.Native.Signal = {};
Elm.Native.Signal.make = function(localRuntime) {

  localRuntime.Native = localRuntime.Native || {};
  localRuntime.Native.Signal = localRuntime.Native.Signal || {};
  if (localRuntime.Native.Signal.values) {
      return localRuntime.Native.Signal.values;
  }

  var Utils = Elm.Native.Utils.make(localRuntime);

  function broadcastToKids(node, timestep, changed) {
    var kids = node.kids;
    for (var i = kids.length; i--; ) {
      kids[i].recv(timestep, changed, node.id);
    }
  }

  function Input(base) {
    this.id = Utils.guid();
    this.value = base;
    this.kids = [];
    this.defaultNumberOfKids = 0;
    this.recv = function(timestep, eid, v) {
      var changed = eid === this.id;
      if (changed) {
        this.value = v;
      }
      broadcastToKids(this, timestep, changed);
      return changed;
    };
    localRuntime.inputs.push(this);
  }

  function LiftN(update, args) {
    this.id = Utils.guid();
    this.value = update();
    this.kids = [];

    var n = args.length;
    var count = 0;
    var isChanged = false;

    this.recv = function(timestep, changed, parentID) {
      ++count;
      if (changed) { isChanged = true; }
      if (count == n) {
        if (isChanged) { this.value = update(); }
        broadcastToKids(this, timestep, isChanged);
        isChanged = false;
        count = 0;
      }
    };
    for (var i = n; i--; ) { args[i].kids.push(this); }
  }

  function map(func, a) {
    function update() { return func(a.value); }
    return new LiftN(update, [a]);
  }
  function map2(func, a, b) {
    function update() { return A2( func, a.value, b.value ); }
    return new LiftN(update, [a,b]);
  }
  function map3(func, a, b, c) {
    function update() { return A3( func, a.value, b.value, c.value ); }
    return new LiftN(update, [a,b,c]);
  }
  function map4(func, a, b, c, d) {
    function update() { return A4( func, a.value, b.value, c.value, d.value ); }
    return new LiftN(update, [a,b,c,d]);
  }
  function map5(func, a, b, c, d, e) {
    function update() { return A5( func, a.value, b.value, c.value, d.value, e.value ); }
    return new LiftN(update, [a,b,c,d,e]);
  }

  function Foldp(step, state, input) {
    this.id = Utils.guid();
    this.value = state;
    this.kids = [];

    this.recv = function(timestep, changed, parentID) {
      if (changed) {
          this.value = A2( step, input.value, this.value );
      }
      broadcastToKids(this, timestep, changed);
    };
    input.kids.push(this);
  }

  function foldp(step, state, input) {
      return new Foldp(step, state, input);
  }

  function DropIf(pred,base,input) {
    this.id = Utils.guid();
    this.value = pred(input.value) ? base : input.value;
    this.kids = [];
    this.recv = function(timestep, changed, parentID) {
      var chng = changed && !pred(input.value);
      if (chng) { this.value = input.value; }
      broadcastToKids(this, timestep, chng);
    };
    input.kids.push(this);
  }

  function DropRepeats(input) {
    this.id = Utils.guid();
    this.value = input.value;
    this.kids = [];
    this.recv = function(timestep, changed, parentID) {
      var chng = changed && !Utils.eq(this.value,input.value);
      if (chng) { this.value = input.value; }
      broadcastToKids(this, timestep, chng);
    };
    input.kids.push(this);
  }

  function timestamp(a) {
    function update() { return Utils.Tuple2(localRuntime.timer.now(), a.value); }
    return new LiftN(update, [a]);
  }

  function SampleOn(s1,s2) {
    this.id = Utils.guid();
    this.value = s2.value;
    this.kids = [];

    var count = 0;
    var isChanged = false;

    this.recv = function(timestep, changed, parentID) {
      if (parentID === s1.id) isChanged = changed;
      ++count;
      if (count == 2) {
        if (isChanged) { this.value = s2.value; }
        broadcastToKids(this, timestep, isChanged);
        count = 0;
        isChanged = false;
      }
    };
    s1.kids.push(this);
    s2.kids.push(this);
  }

  function sampleOn(s1,s2) { return new SampleOn(s1,s2); }

  function delay(t,s) {
      var delayed = new Input(s.value);
      var firstEvent = true;
      function update(v) {
        if (firstEvent) {
            firstEvent = false; return;
        }
        setTimeout(function() {
            localRuntime.notify(delayed.id, v);
        }, t);
      }
      function first(a,b) { return a; }
      return new SampleOn(delayed, map2(F2(first), delayed, map(update,s)));
  }

  function Merge(s1,s2) {
      this.id = Utils.guid();
      this.value = s1.value;
      this.kids = [];

      var next = null;
      var count = 0;
      var isChanged = false;

      this.recv = function(timestep, changed, parentID) {
        ++count;
        if (changed) {
            isChanged = true;
            if (parentID == s2.id && next === null) { next = s2.value; }
            if (parentID == s1.id) { next = s1.value; }
        }

        if (count == 2) {
            if (isChanged) { this.value = next; next = null; }
            broadcastToKids(this, timestep, isChanged);
            isChanged = false;
            count = 0;
        }
      };
      s1.kids.push(this);
      s2.kids.push(this);
  }

  function merge(s1,s2) {
      return new Merge(s1,s2);
  }


    // SIGNAL INPUTS

    function input(initialValue) {
        return new Input(initialValue);
    }

    function send(input, value) {
        return function() {
            localRuntime.notify(input.id, value);
        };
    }

    function subscribe(input) {
        return input;
    }


  return localRuntime.Native.Signal.values = {
    constant : function(v) { return new Input(v); },
    map  : F2(map ),
    map2 : F3(map2),
    map3 : F4(map3),
    map4 : F5(map4),
    map5 : F6(map5),
    foldp : F3(foldp),
    delay : F2(delay),
    merge : F2(merge),
    keepIf : F3(function(pred,base,sig) {
      return new DropIf(function(x) {return !pred(x);},base,sig); }),
    dropIf : F3(function(pred,base,sig) { return new DropIf(pred,base,sig); }),
    dropRepeats : function(s) { return new DropRepeats(s);},
    sampleOn : F2(sampleOn),
    timestamp : timestamp,
    input: input,
    send: F2(send),
    subscribe: subscribe
  };
};

Elm.Native.String = {};
Elm.Native.String.make = function(elm) {
    elm.Native = elm.Native || {};
    elm.Native.String = elm.Native.String || {};
    if (elm.Native.String.values) return elm.Native.String.values;
    if ('values' in Elm.Native.String) {
        return elm.Native.String.values = Elm.Native.String.values;
    }

    var Char = Elm.Char.make(elm);
    var List = Elm.Native.List.make(elm);
    var Maybe = Elm.Maybe.make(elm);
    var Result = Elm.Result.make(elm);
    var Utils = Elm.Native.Utils.make(elm);

    function isEmpty(str) {
        return str.length === 0;
    }
    function cons(chr,str) {
        return chr + str;
    }
    function uncons(str) {
        var hd;
        return (hd = str[0])
            ? Maybe.Just(Utils.Tuple2(Utils.chr(hd), str.slice(1)))
            : Maybe.Nothing;
    }
    function append(a,b) {
        return a + b;
    }
    function concat(strs) {
        return List.toArray(strs).join('');
    }
    function length(str) {
        return str.length;
    }
    function map(f,str) {
        var out = str.split('');
        for (var i = out.length; i--; ) {
            out[i] = f(Utils.chr(out[i]));
        }
        return out.join('');
    }
    function filter(pred,str) {
        return str.split('').map(Utils.chr).filter(pred).join('');
    }
    function reverse(str) {
        return str.split('').reverse().join('');
    }
    function foldl(f,b,str) {
        var len = str.length;
        for (var i = 0; i < len; ++i) {
            b = A2(f, Utils.chr(str[i]), b);
        }
        return b;
    }
    function foldr(f,b,str) {
        for (var i = str.length; i--; ) {
            b = A2(f, Utils.chr(str[i]), b);
        }
        return b;
    }

    function split(sep, str) {
        return List.fromArray(str.split(sep));
    }
    function join(sep, strs) {
        return List.toArray(strs).join(sep);
    }
    function repeat(n, str) {
        var result = '';
        while (n > 0) {
            if (n & 1) result += str;
            n >>= 1, str += str;
        }
        return result;
    }

    function slice(start, end, str) {
        return str.slice(start,end);
    }
    function left(n, str) {
        return n < 1 ? "" : str.slice(0,n);
    }
    function right(n, str) {
        return n < 1 ? "" : str.slice(-n);
    }
    function dropLeft(n, str) {
        return n < 1 ? str : str.slice(n);
    }
    function dropRight(n, str) {
        return n < 1 ? str : str.slice(0,-n);
    }

    function pad(n,chr,str) {
        var half = (n - str.length) / 2;
        return repeat(Math.ceil(half),chr) + str + repeat(half|0,chr);
    }
    function padRight(n,chr,str) {
        return str + repeat(n - str.length, chr);
    }
    function padLeft(n,chr,str) {
        return repeat(n - str.length, chr) + str;
    }

    function trim(str) {
        return str.trim();
    }
    function trimLeft(str) {
        return str.trimLeft();
    }
    function trimRight(str) {
        return str.trimRight();
    }

    function words(str) {
        return List.fromArray(str.trim().split(/\s+/g));
    }
    function lines(str) {
        return List.fromArray(str.split(/\r\n|\r|\n/g));
    }

    function toUpper(str) {
        return str.toUpperCase();
    }
    function toLower(str) {
        return str.toLowerCase();
    }

    function any(pred, str) {
        for (var i = str.length; i--; ) {
            if (pred(Utils.chr(str[i]))) return true;
        }
        return false;
    }
    function all(pred, str) {
        for (var i = str.length; i--; ) {
            if (!pred(Utils.chr(str[i]))) return false;
        }
        return true;
    }

    function contains(sub, str) {
        return str.indexOf(sub) > -1;
    }
    function startsWith(sub, str) {
        return str.indexOf(sub) === 0;
    }
    function endsWith(sub, str) {
        return str.length >= sub.length &&
               str.lastIndexOf(sub) === str.length - sub.length;
    }
    function indexes(sub, str) {
        var subLen = sub.length;
        var i = 0;
        var is = [];
        while ((i = str.indexOf(sub, i)) > -1) {
            is.push(i);
            i = i + subLen;
        }
        return List.fromArray(is);
    }

    function toInt(s) {
        var len = s.length;
        if (len === 0) {
            return Result.Err("could not convert string '" + s + "' to an Int" );
        }
        var start = 0;
        if (s[0] == '-') {
            if (len === 1) {
                return Result.Err("could not convert string '" + s + "' to an Int" );
            }
            start = 1;
        }
        for (var i = start; i < len; ++i) {
            if (!Char.isDigit(s[i])) {
                return Result.Err("could not convert string '" + s + "' to an Int" );
            }
        }
        return Result.Ok(parseInt(s, 10));
    }

    function toFloat(s) {
        var len = s.length;
        if (len === 0) {
            return Result.Err("could not convert string '" + s + "' to a Float" );
        }
        var start = 0;
        if (s[0] == '-') {
            if (len === 1) {
                return Result.Err("could not convert string '" + s + "' to a Float" );
            }
            start = 1;
        }
        var dotCount = 0;
        for (var i = start; i < len; ++i) {
            if (Char.isDigit(s[i])) {
                continue;
            }
            if (s[i] === '.') {
                dotCount += 1;
                if (dotCount <= 1) {
                    continue;
                }
            }
            return Result.Err("could not convert string '" + s + "' to a Float" );
        }
        return Result.Ok(parseFloat(s));
    }

    function toList(str) {
        return List.fromArray(str.split('').map(Utils.chr));
    }
    function fromList(chars) {
        return List.toArray(chars).join('');
    }

    return Elm.Native.String.values = {
        isEmpty: isEmpty,
        cons: F2(cons),
        uncons: uncons,
        append: F2(append),
        concat: concat,
        length: length,
        map: F2(map),
        filter: F2(filter),
        reverse: reverse,
        foldl: F3(foldl),
        foldr: F3(foldr),

        split: F2(split),
        join: F2(join),
        repeat: F2(repeat),

        slice: F3(slice),
        left: F2(left),
        right: F2(right),
        dropLeft: F2(dropLeft),
        dropRight: F2(dropRight),

        pad: F3(pad),
        padLeft: F3(padLeft),
        padRight: F3(padRight),

        trim: trim,
        trimLeft: trimLeft,
        trimRight: trimRight,

        words: words,
        lines: lines,

        toUpper: toUpper,
        toLower: toLower,

        any: F2(any),
        all: F2(all),

        contains: F2(contains),
        startsWith: F2(startsWith),
        endsWith: F2(endsWith),
        indexes: F2(indexes),

        toInt: toInt,
        toFloat: toFloat,
        toList: toList,
        fromList: fromList
    };
};

Elm.Native.Time = {};
Elm.Native.Time.make = function(elm) {

  elm.Native = elm.Native || {};
  elm.Native.Time = elm.Native.Time || {};
  if (elm.Native.Time.values) return elm.Native.Time.values;

  var Signal = Elm.Signal.make(elm);
  var NS = Elm.Native.Signal.make(elm);
  var Maybe = Elm.Maybe.make(elm);
  var Utils = Elm.Native.Utils.make(elm);

  function fpsWhen(desiredFPS, isOn) {
    var msPerFrame = 1000 / desiredFPS;
    var prev = elm.timer.now(), curr = prev, diff = 0, wasOn = true;
    var ticker = NS.input(diff);
    function tick(zero) {
      return function() {
        curr = elm.timer.now();
        diff = zero ? 0 : curr - prev;
        if (prev > curr) {
          diff = 0;
        }
        prev = curr;
        elm.notify(ticker.id, diff);
      };
    }
    var timeoutID = 0;
    function f(isOn, t) {
      if (isOn) {
        timeoutID = elm.setTimeout(tick(!wasOn && isOn), msPerFrame);
      } else if (wasOn) {
        clearTimeout(timeoutID);
      }
      wasOn = isOn;
      return t;
    }
    return A3( Signal.map2, F2(f), isOn, ticker );
  }

  function every(t) {
    var clock = NS.input(elm.timer.now());
    function tellTime() {
        elm.notify(clock.id, elm.timer.now());
    }
    setInterval(tellTime, t);
    return clock;
  }

  function read(s) {
      var t = Date.parse(s);
      return isNaN(t) ? Maybe.Nothing : Maybe.Just(t);
  }
  return elm.Native.Time.values = {
      fpsWhen : F2(fpsWhen),
      fps : function(t) { return fpsWhen(t, Signal.constant(true)); },
      every : every,
      delay : NS.delay,
      timestamp : NS.timestamp,
      toDate : function(t) { return new window.Date(t); },
      read   : read
  };

};

Elm.Native = Elm.Native || {};
Elm.Native.Utils = {};
Elm.Native.Utils.make = function(localRuntime) {

    localRuntime.Native = localRuntime.Native || {};
    localRuntime.Native.Utils = localRuntime.Native.Utils || {};
    if (localRuntime.Native.Utils.values) {
        return localRuntime.Native.Utils.values;
    }

    function eq(l,r) {
        var stack = [{'x': l, 'y': r}]
        while (stack.length > 0) {
            var front = stack.pop();
            var x = front.x;
            var y = front.y;
            if (x === y) continue;
            if (typeof x === "object") {
                var c = 0;
                for (var i in x) {
                    ++c;
                    if (i in y) {
                        if (i !== 'ctor') {
                            stack.push({ 'x': x[i], 'y': y[i] });
                        }
                    } else {
                        return false;
                    }
                }
                if ('ctor' in x) {
                    stack.push({'x': x.ctor, 'y': y.ctor});
                }
                if (c !== Object.keys(y).length) {
                    return false;
                };
            } else if (typeof x === 'function') {
                throw new Error('Equality error: general function equality is ' +
                                'undecidable, and therefore, unsupported');
            } else {
                return false;
            }
        }
        return true;
    }

    // code in Generate/JavaScript.hs depends on the particular
    // integer values assigned to LT, EQ, and GT
    var LT = -1, EQ = 0, GT = 1, ord = ['LT','EQ','GT'];
    function compare(x,y) { return { ctor: ord[cmp(x,y)+1] } }
    function cmp(x,y) {
        var ord;
        if (typeof x !== 'object'){
            return x === y ? EQ : x < y ? LT : GT;
        }
        else if (x.isChar){
            var a = x.toString();
            var b = y.toString();
            return a === b ? EQ : a < b ? LT : GT;
        }
        else if (x.ctor === "::" || x.ctor === "[]") {
            while (true) {
                if (x.ctor === "[]" && y.ctor === "[]") return EQ;
                if (x.ctor !== y.ctor) return x.ctor === '[]' ? LT : GT;
                ord = cmp(x._0, y._0);
                if (ord !== EQ) return ord;
                x = x._1;
                y = y._1;
            }
        }
        else if (x.ctor.slice(0,6) === '_Tuple') {
            var n = x.ctor.slice(6) - 0;
            var err = 'cannot compare tuples with more than 6 elements.';
            if (n === 0) return EQ;
            if (n >= 1) { ord = cmp(x._0, y._0); if (ord !== EQ) return ord;
            if (n >= 2) { ord = cmp(x._1, y._1); if (ord !== EQ) return ord;
            if (n >= 3) { ord = cmp(x._2, y._2); if (ord !== EQ) return ord;
            if (n >= 4) { ord = cmp(x._3, y._3); if (ord !== EQ) return ord;
            if (n >= 5) { ord = cmp(x._4, y._4); if (ord !== EQ) return ord;
            if (n >= 6) { ord = cmp(x._5, y._5); if (ord !== EQ) return ord;
            if (n >= 7) throw new Error('Comparison error: ' + err); } } } } } }
            return EQ;
        }
        else {
            throw new Error('Comparison error: comparison is only defined on ints, ' +
                            'floats, times, chars, strings, lists of comparable values, ' +
                            'and tuples of comparable values.');
        }
    }


    var Tuple0 = { ctor: "_Tuple0" };
    function Tuple2(x,y) {
        return {
            ctor: "_Tuple2",
            _0: x,
            _1: y
        };
    }

    function chr(c) {
        var x = new String(c);
        x.isChar = true;
        return x;
    }

    function txt(str) {
        var t = new String(str);
        t.text = true;
        return t;
    }

    function makeText(text) {
        var style = '';
        var href = '';
        while (true) {
            if (text.style) {
                style += text.style;
                text = text.text;
                continue;
            }
            if (text.href) {
                href = text.href;
                text = text.text;
                continue;
            }
            if (href) {
                text = '<a href="' + href + '">' + text + '</a>';
            }
            if (style) {
                text = '<span style="' + style + '">' + text + '</span>';
            }
            return text;
        }
    }

    var count = 0;
    function guid(_) {
        return count++
    }

    function copy(oldRecord) {
        var newRecord = {};
        for (var key in oldRecord) {
            var value = key === '_'
                ? copy(oldRecord._)
                : oldRecord[key]
                ;
            newRecord[key] = value;
        }
        return newRecord;
    }

    function remove(key, oldRecord) {
        var record = copy(oldRecord);
        if (key in record._) {
            record[key] = record._[key][0];
            record._[key] = record._[key].slice(1);
            if (record._[key].length === 0) {
                delete record._[key];
            }
        } else {
            delete record[key];
        }
        return record;
    }

    function replace(keyValuePairs, oldRecord) {
        var record = copy(oldRecord);
        for (var i = keyValuePairs.length; i--; ) {
            var pair = keyValuePairs[i];
            record[pair[0]] = pair[1];
        }
        return record;
    }

    function insert(key, value, oldRecord) {
        var newRecord = copy(oldRecord);
        if (key in newRecord) {
            var values = newRecord._[key];
            var copiedValues = values ? values.slice(0) : [];
            newRecord._[key] = [newRecord[key]].concat(copiedValues);
        }
        newRecord[key] = value;
        return newRecord;
    }

    function getXY(e) {
        var posx = 0;
        var posy = 0;
        if (e.pageX || e.pageY) {
            posx = e.pageX;
            posy = e.pageY;
        } else if (e.clientX || e.clientY) {
            posx = e.clientX + document.body.scrollLeft + document.documentElement.scrollLeft;
            posy = e.clientY + document.body.scrollTop + document.documentElement.scrollTop;
        }

        if (localRuntime.isEmbed()) {
            var rect = localRuntime.node.getBoundingClientRect();
            var relx = rect.left + document.body.scrollLeft + document.documentElement.scrollLeft;
            var rely = rect.top + document.body.scrollTop + document.documentElement.scrollTop;
            // TODO: figure out if there is a way to avoid rounding here
            posx = posx - Math.round(relx) - localRuntime.node.clientLeft;
            posy = posy - Math.round(rely) - localRuntime.node.clientTop;
        }
        return Tuple2(posx, posy);
    }


    //// LIST STUFF ////

    var Nil = { ctor:'[]' };

    function Cons(hd,tl) {
        return {
            ctor: "::",
            _0: hd,
            _1: tl
        };
    }

    function append(xs,ys) {
        // append Text
        if (xs.text || ys.text) {
            return txt(makeText(xs) + makeText(ys));
        }

        // append Strings
        if (typeof xs === "string") {
            return xs + ys;
        }

        // append Lists
        if (xs.ctor === '[]') {
            return ys;
        }
        var root = Cons(xs._0, Nil);
        var curr = root;
        xs = xs._1;
        while (xs.ctor !== '[]') {
            curr._1 = Cons(xs._0, Nil);
            xs = xs._1;
            curr = curr._1;
        }
        curr._1 = ys;
        return root;
    }

    //// RUNTIME ERRORS ////

    function indent(lines) {
        return '\n' + lines.join('\n');
    }

    function badCase(moduleName, span) { 
        var msg = indent([
            'Non-exhaustive pattern match in case-expression.',
            'Make sure your patterns cover every case!'
        ]);
        throw new Error('Runtime error in module ' + moduleName + ' (' + span + ')' + msg);
    }

    function badIf(moduleName, span) { 
        var msg = indent([
            'Non-exhaustive pattern match in multi-way-if expression.',
            'It is best to use \'otherwise\' as the last branch of multi-way-if.'
        ]);
        throw new Error('Runtime error in module ' + moduleName + ' (' + span + ')' + msg);
    }


    function badPort(expected, received) { 
        var msg = indent([
            'Expecting ' + expected + ' but was given ',
            JSON.stringify(received)
        ]);
        throw new Error('Runtime error when sending values through a port.' + msg);
    }


    return localRuntime.Native.Utils.values = {
        eq:eq,
        cmp:cmp,
        compare:F2(compare),
        Tuple0:Tuple0,
        Tuple2:Tuple2,
        chr:chr,
        txt:txt,
        makeText:makeText,
        copy: copy,
        remove: remove,
        replace: replace,
        insert: insert,
        guid: guid,
        getXY: getXY,

        Nil: Nil,
        Cons: Cons,
        append: F2(append),

        badCase: badCase,
        badIf: badIf,
        badPort: badPort
    };
};

Elm.Result = Elm.Result || {};
Elm.Result.make = function (_elm) {
   "use strict";
   _elm.Result = _elm.Result || {};
   if (_elm.Result.values)
   return _elm.Result.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Result",
   $Maybe = Elm.Maybe.make(_elm);
   var toMaybe = function (result) {
      return function () {
         switch (result.ctor)
         {case "Err":
            return $Maybe.Nothing;
            case "Ok":
            return $Maybe.Just(result._0);}
         _U.badCase($moduleName,
         "between lines 158 and 171");
      }();
   };
   var Err = function (a) {
      return {ctor: "Err",_0: a};
   };
   var andThen = F2(function (result,
   callback) {
      return function () {
         switch (result.ctor)
         {case "Err":
            return Err(result._0);
            case "Ok":
            return callback(result._0);}
         _U.badCase($moduleName,
         "between lines 120 and 139");
      }();
   });
   var Ok = function (a) {
      return {ctor: "Ok",_0: a};
   };
   var map = F2(function (func,
   ra) {
      return function () {
         switch (ra.ctor)
         {case "Err": return Err(ra._0);
            case "Ok":
            return Ok(func(ra._0));}
         _U.badCase($moduleName,
         "between lines 35 and 46");
      }();
   });
   var map2 = F3(function (func,
   ra,
   rb) {
      return function () {
         var _v9 = {ctor: "_Tuple2"
                   ,_0: ra
                   ,_1: rb};
         switch (_v9.ctor)
         {case "_Tuple2":
            switch (_v9._0.ctor)
              {case "Err":
                 return Err(_v9._0._0);
                 case "Ok": switch (_v9._1.ctor)
                   {case "Ok": return Ok(A2(func,
                        _v9._0._0,
                        _v9._1._0));}
                   break;}
              switch (_v9._1.ctor)
              {case "Err":
                 return Err(_v9._1._0);}
              break;}
         _U.badCase($moduleName,
         "between lines 49 and 52");
      }();
   });
   var map3 = F4(function (func,
   ra,
   rb,
   rc) {
      return function () {
         var _v16 = {ctor: "_Tuple3"
                    ,_0: ra
                    ,_1: rb
                    ,_2: rc};
         switch (_v16.ctor)
         {case "_Tuple3":
            switch (_v16._0.ctor)
              {case "Err":
                 return Err(_v16._0._0);
                 case "Ok": switch (_v16._1.ctor)
                   {case "Ok":
                      switch (_v16._2.ctor)
                        {case "Ok": return Ok(A3(func,
                             _v16._0._0,
                             _v16._1._0,
                             _v16._2._0));}
                        break;}
                   break;}
              switch (_v16._1.ctor)
              {case "Err":
                 return Err(_v16._1._0);}
              switch (_v16._2.ctor)
              {case "Err":
                 return Err(_v16._2._0);}
              break;}
         _U.badCase($moduleName,
         "between lines 57 and 61");
      }();
   });
   var map4 = F5(function (func,
   ra,
   rb,
   rc,
   rd) {
      return function () {
         var _v26 = {ctor: "_Tuple4"
                    ,_0: ra
                    ,_1: rb
                    ,_2: rc
                    ,_3: rd};
         switch (_v26.ctor)
         {case "_Tuple4":
            switch (_v26._0.ctor)
              {case "Err":
                 return Err(_v26._0._0);
                 case "Ok": switch (_v26._1.ctor)
                   {case "Ok":
                      switch (_v26._2.ctor)
                        {case "Ok":
                           switch (_v26._3.ctor)
                             {case "Ok": return Ok(A4(func,
                                  _v26._0._0,
                                  _v26._1._0,
                                  _v26._2._0,
                                  _v26._3._0));}
                             break;}
                        break;}
                   break;}
              switch (_v26._1.ctor)
              {case "Err":
                 return Err(_v26._1._0);}
              switch (_v26._2.ctor)
              {case "Err":
                 return Err(_v26._2._0);}
              switch (_v26._3.ctor)
              {case "Err":
                 return Err(_v26._3._0);}
              break;}
         _U.badCase($moduleName,
         "between lines 66 and 71");
      }();
   });
   var map5 = F6(function (func,
   ra,
   rb,
   rc,
   rd,
   re) {
      return function () {
         var _v39 = {ctor: "_Tuple5"
                    ,_0: ra
                    ,_1: rb
                    ,_2: rc
                    ,_3: rd
                    ,_4: re};
         switch (_v39.ctor)
         {case "_Tuple5":
            switch (_v39._0.ctor)
              {case "Err":
                 return Err(_v39._0._0);
                 case "Ok": switch (_v39._1.ctor)
                   {case "Ok":
                      switch (_v39._2.ctor)
                        {case "Ok":
                           switch (_v39._3.ctor)
                             {case "Ok":
                                switch (_v39._4.ctor)
                                  {case "Ok": return Ok(A5(func,
                                       _v39._0._0,
                                       _v39._1._0,
                                       _v39._2._0,
                                       _v39._3._0,
                                       _v39._4._0));}
                                  break;}
                             break;}
                        break;}
                   break;}
              switch (_v39._1.ctor)
              {case "Err":
                 return Err(_v39._1._0);}
              switch (_v39._2.ctor)
              {case "Err":
                 return Err(_v39._2._0);}
              switch (_v39._3.ctor)
              {case "Err":
                 return Err(_v39._3._0);}
              switch (_v39._4.ctor)
              {case "Err":
                 return Err(_v39._4._0);}
              break;}
         _U.badCase($moduleName,
         "between lines 76 and 117");
      }();
   });
   var formatError = F2(function (f,
   result) {
      return function () {
         switch (result.ctor)
         {case "Err":
            return Err(f(result._0));
            case "Ok":
            return Ok(result._0);}
         _U.badCase($moduleName,
         "between lines 142 and 155");
      }();
   });
   var fromMaybe = F2(function (err,
   maybe) {
      return function () {
         switch (maybe.ctor)
         {case "Just":
            return Ok(maybe._0);
            case "Nothing":
            return Err(err);}
         _U.badCase($moduleName,
         "between lines 174 and 176");
      }();
   });
   _elm.Result.values = {_op: _op
                        ,Ok: Ok
                        ,Err: Err
                        ,map: map
                        ,map2: map2
                        ,map3: map3
                        ,map4: map4
                        ,map5: map5
                        ,andThen: andThen
                        ,formatError: formatError
                        ,toMaybe: toMaybe
                        ,fromMaybe: fromMaybe};
   return _elm.Result.values;
};
Elm.Signal = Elm.Signal || {};
Elm.Signal.make = function (_elm) {
   "use strict";
   _elm.Signal = _elm.Signal || {};
   if (_elm.Signal.values)
   return _elm.Signal.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Signal",
   $Basics = Elm.Basics.make(_elm),
   $List = Elm.List.make(_elm),
   $Native$Signal = Elm.Native.Signal.make(_elm);
   var subscribe = $Native$Signal.subscribe;
   var send = $Native$Signal.send;
   var channel = $Native$Signal.input;
   var Message = {ctor: "Message"};
   var Channel = {ctor: "Channel"};
   _op["~"] = F2(function (sf,s) {
      return A3($Native$Signal.map2,
      F2(function (f,x) {
         return f(x);
      }),
      sf,
      s);
   });
   _op["<~"] = F2(function (f,s) {
      return A2($Native$Signal.map,
      f,
      s);
   });
   var sampleOn = $Native$Signal.sampleOn;
   var dropRepeats = $Native$Signal.dropRepeats;
   var dropIf = $Native$Signal.dropIf;
   var keepIf = $Native$Signal.keepIf;
   var keepWhen = F3(function (bs,
   def,
   sig) {
      return A2(_op["<~"],
      $Basics.snd,
      A3(keepIf,
      $Basics.fst,
      {ctor: "_Tuple2"
      ,_0: false
      ,_1: def},
      A2(_op["~"],
      A2(_op["<~"],
      F2(function (v0,v1) {
         return {ctor: "_Tuple2"
                ,_0: v0
                ,_1: v1};
      }),
      A2(sampleOn,sig,bs)),
      sig)));
   });
   var dropWhen = function (bs) {
      return keepWhen(A2(_op["<~"],
      $Basics.not,
      bs));
   };
   var merge = $Native$Signal.merge;
   var mergeMany = function (signals) {
      return A2($List.foldr1,
      merge,
      signals);
   };
   var foldp = $Native$Signal.foldp;
   var map5 = $Native$Signal.map5;
   var map4 = $Native$Signal.map4;
   var map3 = $Native$Signal.map3;
   var map2 = $Native$Signal.map2;
   var map = $Native$Signal.map;
   var constant = $Native$Signal.constant;
   var Signal = {ctor: "Signal"};
   _elm.Signal.values = {_op: _op
                        ,Signal: Signal
                        ,constant: constant
                        ,map: map
                        ,map2: map2
                        ,map3: map3
                        ,map4: map4
                        ,map5: map5
                        ,foldp: foldp
                        ,merge: merge
                        ,mergeMany: mergeMany
                        ,keepIf: keepIf
                        ,dropIf: dropIf
                        ,keepWhen: keepWhen
                        ,dropWhen: dropWhen
                        ,dropRepeats: dropRepeats
                        ,sampleOn: sampleOn
                        ,Channel: Channel
                        ,Message: Message
                        ,channel: channel
                        ,send: send
                        ,subscribe: subscribe};
   return _elm.Signal.values;
};
Elm.String = Elm.String || {};
Elm.String.make = function (_elm) {
   "use strict";
   _elm.String = _elm.String || {};
   if (_elm.String.values)
   return _elm.String.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "String",
   $Maybe = Elm.Maybe.make(_elm),
   $Native$String = Elm.Native.String.make(_elm),
   $Result = Elm.Result.make(_elm);
   var fromList = $Native$String.fromList;
   var toList = $Native$String.toList;
   var toFloat = $Native$String.toFloat;
   var toInt = $Native$String.toInt;
   var indices = $Native$String.indexes;
   var indexes = $Native$String.indexes;
   var endsWith = $Native$String.endsWith;
   var startsWith = $Native$String.startsWith;
   var contains = $Native$String.contains;
   var all = $Native$String.all;
   var any = $Native$String.any;
   var toLower = $Native$String.toLower;
   var toUpper = $Native$String.toUpper;
   var lines = $Native$String.lines;
   var words = $Native$String.words;
   var trimRight = $Native$String.trimRight;
   var trimLeft = $Native$String.trimLeft;
   var trim = $Native$String.trim;
   var padRight = $Native$String.padRight;
   var padLeft = $Native$String.padLeft;
   var pad = $Native$String.pad;
   var dropRight = $Native$String.dropRight;
   var dropLeft = $Native$String.dropLeft;
   var right = $Native$String.right;
   var left = $Native$String.left;
   var slice = $Native$String.slice;
   var repeat = $Native$String.repeat;
   var join = $Native$String.join;
   var split = $Native$String.split;
   var foldr = $Native$String.foldr;
   var foldl = $Native$String.foldl;
   var reverse = $Native$String.reverse;
   var filter = $Native$String.filter;
   var map = $Native$String.map;
   var length = $Native$String.length;
   var concat = $Native$String.concat;
   var append = $Native$String.append;
   var uncons = $Native$String.uncons;
   var cons = $Native$String.cons;
   var fromChar = function ($char) {
      return A2(cons,$char,"");
   };
   var isEmpty = $Native$String.isEmpty;
   _elm.String.values = {_op: _op
                        ,isEmpty: isEmpty
                        ,cons: cons
                        ,fromChar: fromChar
                        ,uncons: uncons
                        ,append: append
                        ,concat: concat
                        ,length: length
                        ,map: map
                        ,filter: filter
                        ,reverse: reverse
                        ,foldl: foldl
                        ,foldr: foldr
                        ,split: split
                        ,join: join
                        ,repeat: repeat
                        ,slice: slice
                        ,left: left
                        ,right: right
                        ,dropLeft: dropLeft
                        ,dropRight: dropRight
                        ,pad: pad
                        ,padLeft: padLeft
                        ,padRight: padRight
                        ,trim: trim
                        ,trimLeft: trimLeft
                        ,trimRight: trimRight
                        ,words: words
                        ,lines: lines
                        ,toUpper: toUpper
                        ,toLower: toLower
                        ,any: any
                        ,all: all
                        ,contains: contains
                        ,startsWith: startsWith
                        ,endsWith: endsWith
                        ,indexes: indexes
                        ,indices: indices
                        ,toInt: toInt
                        ,toFloat: toFloat
                        ,toList: toList
                        ,fromList: fromList};
   return _elm.String.values;
};
Elm.Time = Elm.Time || {};
Elm.Time.make = function (_elm) {
   "use strict";
   _elm.Time = _elm.Time || {};
   if (_elm.Time.values)
   return _elm.Time.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Time",
   $Basics = Elm.Basics.make(_elm),
   $Native$Time = Elm.Native.Time.make(_elm),
   $Signal = Elm.Signal.make(_elm);
   var delay = $Native$Time.delay;
   var timestamp = $Native$Time.timestamp;
   var since = F2(function (t,s) {
      return function () {
         var stop = A2($Signal.map,
         $Basics.always(-1),
         A2(delay,t,s));
         var start = A2($Signal.map,
         $Basics.always(1),
         s);
         var delaydiff = A3($Signal.foldp,
         F2(function (x,y) {
            return x + y;
         }),
         0,
         A2($Signal.merge,start,stop));
         return A2($Signal.map,
         F2(function (x,y) {
            return !_U.eq(x,y);
         })(0),
         delaydiff);
      }();
   });
   var every = $Native$Time.every;
   var fpsWhen = $Native$Time.fpsWhen;
   var fps = $Native$Time.fps;
   var inMilliseconds = function (t) {
      return t;
   };
   var millisecond = 1;
   var second = 1000 * millisecond;
   var minute = 60 * second;
   var hour = 60 * minute;
   var inHours = function (t) {
      return t / hour;
   };
   var inMinutes = function (t) {
      return t / minute;
   };
   var inSeconds = function (t) {
      return t / second;
   };
   _elm.Time.values = {_op: _op
                      ,millisecond: millisecond
                      ,second: second
                      ,minute: minute
                      ,hour: hour
                      ,inMilliseconds: inMilliseconds
                      ,inSeconds: inSeconds
                      ,inMinutes: inMinutes
                      ,inHours: inHours
                      ,fps: fps
                      ,fpsWhen: fpsWhen
                      ,every: every
                      ,since: since
                      ,timestamp: timestamp
                      ,delay: delay};
   return _elm.Time.values;
};
Elm.ToGuiMessage = Elm.ToGuiMessage || {};
Elm.ToGuiMessage.make = function (_elm) {
   "use strict";
   _elm.ToGuiMessage = _elm.ToGuiMessage || {};
   if (_elm.ToGuiMessage.values)
   return _elm.ToGuiMessage.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "ToGuiMessage",
   $CommonState = Elm.CommonState.make(_elm);
   var decode = function (msg) {
      return function () {
         var setConnected = function () {
            var _v0 = msg.setConnected;
            switch (_v0)
            {case 0:
               return $CommonState.NotConnected;
               case 1:
               return $CommonState.Connected;
               case 2:
               return $CommonState.NoCard;
               case 3:
               return $CommonState.NoPin;}
            _U.badCase($moduleName,
            "between lines 23 and 28");
         }();
         return _L.fromArray([$CommonState.SetLog(msg.setLog)
                             ,$CommonState.SetConnected(setConnected)]);
      }();
   };
   var encode = function (com) {
      return {_: {}
             ,setConnected: function () {
                var _v1 = com.connected;
                switch (_v1.ctor)
                {case "Connected": return 1;
                   case "NoCard": return 2;
                   case "NoPin": return 3;
                   case "NotConnected": return 0;}
                _U.badCase($moduleName,
                "between lines 13 and 18");
             }()
             ,setLog: com.log};
   };
   var ToGuiMessage = F2(function (a,
   b) {
      return {_: {}
             ,setConnected: b
             ,setLog: a};
   });
   _elm.ToGuiMessage.values = {_op: _op
                              ,ToGuiMessage: ToGuiMessage
                              ,encode: encode
                              ,decode: decode};
   return _elm.ToGuiMessage.values;
};
Elm.Util = Elm.Util || {};
Elm.Util.make = function (_elm) {
   "use strict";
   _elm.Util = _elm.Util || {};
   if (_elm.Util.values)
   return _elm.Util.values;
   var _op = {},
   _N = Elm.Native,
   _U = _N.Utils.make(_elm),
   _L = _N.List.make(_elm),
   _P = _N.Ports.make(_elm),
   $moduleName = "Util",
   $Maybe = Elm.Maybe.make(_elm);
   var isJust = function (x) {
      return function () {
         switch (x.ctor)
         {case "Just": return true;
            case "Nothing": return false;}
         _U.badCase($moduleName,
         "between lines 4 and 6");
      }();
   };
   _elm.Util.values = {_op: _op
                      ,isJust: isJust};
   return _elm.Util.values;
};
