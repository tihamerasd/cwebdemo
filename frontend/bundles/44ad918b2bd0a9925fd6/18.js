(window.webpackJsonp=window.webpackJsonp||[]).push([[18],{1452:function(e,n,t){"use strict";t.r(n),t.d(n,"default",(function(){return p}));var o=t(5),s=t.n(o),i=t(0),a=t.n(i),r=t(2),u=t.n(r),c=t(3),l=t(1);class p extends a.a.PureComponent{constructor(...e){super(...e),s()(this,"onDontAskAgainClick",()=>{this.props.onFinished(),this.props.onDontAskAgain()}),s()(this,"onSetupClick",()=>{this.props.onFinished(),this.props.onSetup()})}render(){const e=c.a("views.dialogs.BaseDialog"),n=c.a("views.elements.DialogButtons");return a.a.createElement(e,{className:"mx_IgnoreRecoveryReminderDialog",onFinished:this.props.onFinished,title:Object(l.a)("Are you sure?")},a.a.createElement("div",null,a.a.createElement("p",null,Object(l.a)("Without setting up Secure Message Recovery, you'll lose your secure message history when you log out.")),a.a.createElement("p",null,Object(l.a)("If you don't want to set this up now, you can later in Settings.")),a.a.createElement("div",{className:"mx_Dialog_buttons"},a.a.createElement(n,{primaryButton:Object(l.a)("Set up"),onPrimaryButtonClick:this.onSetupClick,cancelButton:Object(l.a)("Don't ask again"),onCancel:this.onDontAskAgainClick}))))}}s()(p,"propTypes",{onDontAskAgain:u.a.func.isRequired,onFinished:u.a.func.isRequired,onSetup:u.a.func.isRequired})}}]);