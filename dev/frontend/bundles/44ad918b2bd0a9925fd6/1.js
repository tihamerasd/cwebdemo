/*! For license information please see 1.js.LICENSE.txt */
(window.webpackJsonp=window.webpackJsonp||[]).push([[1],{1432:function(e,t,a){var s,n=n||function(e){"use strict";if(!(void 0===e||"undefined"!=typeof navigator&&/MSIE [1-9]\./.test(navigator.userAgent))){var t=e.document,a=function(){return e.URL||e.webkitURL||e},s=t.createElementNS("http://www.w3.org/1999/xhtml","a"),n="download"in s,r=/constructor/i.test(e.HTMLElement)||e.safari,o=/CriOS\/[\d]+/.test(navigator.userAgent),i=function(t){(e.setImmediate||e.setTimeout)((function(){throw t}),0)},c=function(e){setTimeout((function(){"string"==typeof e?a().revokeObjectURL(e):e.remove()}),4e4)},l=function(e){return/^\s*(?:text\/\S*|application\/xml|\S*\/\S*\+xml)\s*;.*charset\s*=\s*utf-8/i.test(e.type)?new Blob([String.fromCharCode(65279),e],{type:e.type}):e},u=function(t,u,h){h||(t=l(t));var p,m=this,d="application/octet-stream"===t.type,y=function(){!function(e,t,a){for(var s=(t=[].concat(t)).length;s--;){var n=e["on"+t[s]];if("function"==typeof n)try{n.call(e,a||e)}catch(e){i(e)}}}(m,"writestart progress write writeend".split(" "))};if(m.readyState=m.INIT,n)return p=a().createObjectURL(t),void setTimeout((function(){var e,t;s.href=p,s.download=u,e=s,t=new MouseEvent("click"),e.dispatchEvent(t),y(),c(p),m.readyState=m.DONE}));!function(){if((o||d&&r)&&e.FileReader){var s=new FileReader;return s.onloadend=function(){var t=o?s.result:s.result.replace(/^data:[^;]*;/,"data:attachment/file;");e.open(t,"_blank")||(e.location.href=t),t=void 0,m.readyState=m.DONE,y()},s.readAsDataURL(t),void(m.readyState=m.INIT)}(p||(p=a().createObjectURL(t)),d)?e.location.href=p:e.open(p,"_blank")||(e.location.href=p);m.readyState=m.DONE,y(),c(p)}()},h=u.prototype;return"undefined"!=typeof navigator&&navigator.msSaveOrOpenBlob?function(e,t,a){return t=t||e.name||"download",a||(e=l(e)),navigator.msSaveOrOpenBlob(e,t)}:(h.abort=function(){},h.readyState=h.INIT=0,h.WRITING=1,h.DONE=2,h.error=h.onwritestart=h.onprogress=h.onwrite=h.onabort=h.onerror=h.onwriteend=null,function(e,t,a){return new u(e,t||e.name||"download",a)})}}("undefined"!=typeof self&&self||"undefined"!=typeof window&&window||this.content);e.exports?e.exports.saveAs=n:null!==a(1433)&&null!==a(1434)&&(void 0===(s=function(){return n}.call(t,a,t,e))||(e.exports=s))},1433:function(e,t){e.exports=function(){throw new Error("define cannot be used indirect")}},1434:function(e,t){(function(t){e.exports=t}).call(this,{})},1439:function(e,t,a){"use strict";a.r(t),a.d(t,"default",(function(){return f}));var s=a(5),n=a.n(s),r=a(0),o=a.n(r),i=a(1432),c=a.n(i),l=a(3),u=a(4),h=a(2),p=a.n(h),m=a(1437),d=a(1),y=a(91),b=a(7),k=a(12);const v=0,_=5;class f extends o.a.PureComponent{constructor(e){super(e),n()(this,"_collectRecoveryKeyNode",e=>{this._recoveryKeyNode=e}),n()(this,"_onCopyClick",()=>{!function(e){const t=document.createRange();t.selectNodeContents(e);const a=window.getSelection();a.removeAllRanges(),a.addRange(t)}(this._recoveryKeyNode),document.execCommand("copy")&&this.setState({copied:!0,phase:3})}),n()(this,"_onDownloadClick",()=>{const e=new Blob([this._keyBackupInfo.recovery_key],{type:"text/plain;charset=us-ascii"});c.a.saveAs(e,"recovery-key.txt"),this.setState({downloaded:!0,phase:3})}),n()(this,"_createBackup",async()=>{const{secureSecretStorage:e}=this.state;let t;this.setState({phase:4,error:null});try{e?await Object(y.b)(async()=>{t=await u.a.get().prepareKeyBackupVersion(null,{secureSecretStorage:!0}),t=await u.a.get().createKeyBackupVersion(t)}):t=await u.a.get().createKeyBackupVersion(this._keyBackupInfo),await u.a.get().scheduleAllGroupSessionsForBackup(),this.setState({phase:_})}catch(e){console.error("Error creating key backup",e),t&&u.a.get().deleteKeyBackupVersion(t.version),this.setState({error:e})}}),n()(this,"_onCancel",()=>{this.props.onFinished(!1)}),n()(this,"_onDone",()=>{this.props.onFinished(!0)}),n()(this,"_onOptOutClick",()=>{this.setState({phase:6})}),n()(this,"_onSetUpClick",()=>{this.setState({phase:v})}),n()(this,"_onSkipPassPhraseClick",async()=>{this._keyBackupInfo=await u.a.get().prepareKeyBackupVersion(),this.setState({copied:!1,downloaded:!1,phase:2})}),n()(this,"_onPassPhraseNextClick",async e=>{e.preventDefault(),null!==this._setZxcvbnResultTimeout&&(clearTimeout(this._setZxcvbnResultTimeout),this._setZxcvbnResultTimeout=null,await new Promise(e=>{this.setState({zxcvbnResult:Object(m.scorePassword)(this.state.passPhrase)},e)})),this._passPhraseIsValid()&&this.setState({phase:1})}),n()(this,"_onPassPhraseConfirmNextClick",async e=>{e.preventDefault(),this.state.passPhrase===this.state.passPhraseConfirm&&(this._keyBackupInfo=await u.a.get().prepareKeyBackupVersion(this.state.passPhrase),this.setState({copied:!1,downloaded:!1,phase:2}))}),n()(this,"_onSetAgainClick",()=>{this.setState({passPhrase:"",passPhraseConfirm:"",phase:v,zxcvbnResult:null})}),n()(this,"_onKeepItSafeBackClick",()=>{this.setState({phase:2})}),n()(this,"_onPassPhraseChange",e=>{this.setState({passPhrase:e.target.value}),null!==this._setZxcvbnResultTimeout&&clearTimeout(this._setZxcvbnResultTimeout),this._setZxcvbnResultTimeout=setTimeout(()=>{this._setZxcvbnResultTimeout=null,this.setState({zxcvbnResult:Object(m.scorePassword)(this.state.passPhrase)})},500)}),n()(this,"_onPassPhraseConfirmChange",e=>{this.setState({passPhraseConfirm:e.target.value})}),this._recoveryKeyNode=null,this._keyBackupInfo=null,this._setZxcvbnResultTimeout=null,this.state={secureSecretStorage:null,phase:v,passPhrase:"",passPhraseConfirm:"",copied:!1,downloaded:!1,zxcvbnResult:null}}async componentDidMount(){const e=u.a.get(),t=b.b.isFeatureEnabled("feature_cross_signing")&&await e.doesServerSupportUnstableFeature("org.matrix.e2e_cross_signing");this.setState({secureSecretStorage:t}),t&&(this.setState({phase:4}),this._createBackup())}componentWillUnmount(){null!==this._setZxcvbnResultTimeout&&clearTimeout(this._setZxcvbnResultTimeout)}_passPhraseIsValid(){return this.state.zxcvbnResult&&this.state.zxcvbnResult.score>=4}_renderPhasePassPhrase(){const e=l.a("views.elements.DialogButtons");let t,a;if(this.state.zxcvbnResult){if(this.state.zxcvbnResult.score>=4)a=Object(d.a)("Great! This passphrase looks strong enough.");else{const e=[];for(let t=0;t<this.state.zxcvbnResult.feedback.suggestions.length;++t)e.push(o.a.createElement("div",{key:t},this.state.zxcvbnResult.feedback.suggestions[t]));const t=o.a.createElement("div",null,e.length>0?e:Object(d.a)("Keep going..."));a=o.a.createElement("div",null,this.state.zxcvbnResult.feedback.warning,t)}t=o.a.createElement("div",null,o.a.createElement("progress",{max:4,value:this.state.zxcvbnResult.score}))}return o.a.createElement("form",{onSubmit:this._onPassPhraseNextClick},o.a.createElement("p",null,Object(d.a)("<b>Warning</b>: You should only set up key backup from a trusted computer.",{},{b:e=>o.a.createElement("b",null,e)})),o.a.createElement("p",null,Object(d.a)("We'll store an encrypted copy of your keys on our server. Protect your backup with a passphrase to keep it secure.")),o.a.createElement("p",null,Object(d.a)("For maximum security, this should be different from your account password.")),o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_primaryContainer"},o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_passPhraseContainer"},o.a.createElement("input",{type:"password",onChange:this._onPassPhraseChange,value:this.state.passPhrase,className:"mx_CreateKeyBackupDialog_passPhraseInput",placeholder:Object(d.a)("Enter a passphrase..."),autoFocus:!0}),o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_passPhraseHelp"},t,a))),o.a.createElement(e,{primaryButton:Object(d.a)("Next"),onPrimaryButtonClick:this._onPassPhraseNextClick,hasCancel:!1,disabled:!this._passPhraseIsValid()}),o.a.createElement("details",null,o.a.createElement("summary",null,Object(d.a)("Advanced")),o.a.createElement(k.a,{kind:"primary",onClick:this._onSkipPassPhraseClick},Object(d.a)("Set up with a recovery key"))))}_renderPhasePassPhraseConfirm(){const e=l.a("elements.AccessibleButton");let t;this.state.passPhraseConfirm===this.state.passPhrase?t=Object(d.a)("That matches!"):this.state.passPhrase.startsWith(this.state.passPhraseConfirm)||(t=Object(d.a)("That doesn't match."));let a=null;t&&(a=o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_passPhraseMatch"},o.a.createElement("div",null,t),o.a.createElement("div",null,o.a.createElement(e,{element:"span",className:"mx_linkButton",onClick:this._onSetAgainClick},Object(d.a)("Go back to set it again.")))));const s=l.a("views.elements.DialogButtons");return o.a.createElement("form",{onSubmit:this._onPassPhraseConfirmNextClick},o.a.createElement("p",null,Object(d.a)("Please enter your passphrase a second time to confirm.")),o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_primaryContainer"},o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_passPhraseContainer"},o.a.createElement("div",null,o.a.createElement("input",{type:"password",onChange:this._onPassPhraseConfirmChange,value:this.state.passPhraseConfirm,className:"mx_CreateKeyBackupDialog_passPhraseInput",placeholder:Object(d.a)("Repeat your passphrase..."),autoFocus:!0})),a)),o.a.createElement(s,{primaryButton:Object(d.a)("Next"),onPrimaryButtonClick:this._onPassPhraseConfirmNextClick,hasCancel:!1,disabled:this.state.passPhrase!==this.state.passPhraseConfirm}))}_renderPhaseShowKey(){return o.a.createElement("div",null,o.a.createElement("p",null,Object(d.a)("Your recovery key is a safety net - you can use it to restore access to your encrypted messages if you forget your passphrase.")),o.a.createElement("p",null,Object(d.a)("Keep a copy of it somewhere secure, like a password manager or even a safe.")),o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_primaryContainer"},o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_recoveryKeyHeader"},Object(d.a)("Your recovery key")),o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_recoveryKeyContainer"},o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_recoveryKey"},o.a.createElement("code",{ref:this._collectRecoveryKeyNode},this._keyBackupInfo.recovery_key)),o.a.createElement("div",{className:"mx_CreateKeyBackupDialog_recoveryKeyButtons"},o.a.createElement("button",{className:"mx_Dialog_primary",onClick:this._onCopyClick},Object(d.a)("Copy")),o.a.createElement("button",{className:"mx_Dialog_primary",onClick:this._onDownloadClick},Object(d.a)("Download"))))))}_renderPhaseKeepItSafe(){let e;this.state.copied?e=Object(d.a)("Your recovery key has been <b>copied to your clipboard</b>, paste it to:",{},{b:e=>o.a.createElement("b",null,e)}):this.state.downloaded&&(e=Object(d.a)("Your recovery key is in your <b>Downloads</b> folder.",{},{b:e=>o.a.createElement("b",null,e)}));const t=l.a("views.elements.DialogButtons");return o.a.createElement("div",null,e,o.a.createElement("ul",null,o.a.createElement("li",null,Object(d.a)("<b>Print it</b> and store it somewhere safe",{},{b:e=>o.a.createElement("b",null,e)})),o.a.createElement("li",null,Object(d.a)("<b>Save it</b> on a USB key or backup drive",{},{b:e=>o.a.createElement("b",null,e)})),o.a.createElement("li",null,Object(d.a)("<b>Copy it</b> to your personal cloud storage",{},{b:e=>o.a.createElement("b",null,e)}))),o.a.createElement(t,{primaryButton:Object(d.a)("Continue"),onPrimaryButtonClick:this._createBackup,hasCancel:!1},o.a.createElement("button",{onClick:this._onKeepItSafeBackClick},Object(d.a)("Back"))))}_renderBusyPhase(e){const t=l.a("views.elements.Spinner");return o.a.createElement("div",null,o.a.createElement(t,null))}_renderPhaseDone(){const e=l.a("views.elements.DialogButtons");return o.a.createElement("div",null,o.a.createElement("p",null,Object(d.a)("Your keys are being backed up (the first backup could take a few minutes).")),o.a.createElement(e,{primaryButton:Object(d.a)("OK"),onPrimaryButtonClick:this._onDone,hasCancel:!1}))}_renderPhaseOptOutConfirm(){const e=l.a("views.elements.DialogButtons");return o.a.createElement("div",null,Object(d.a)("Without setting up Secure Message Recovery, you won't be able to restore your encrypted message history if you log out or use another session."),o.a.createElement(e,{primaryButton:Object(d.a)("Set up Secure Message Recovery"),onPrimaryButtonClick:this._onSetUpClick,hasCancel:!1},o.a.createElement("button",{onClick:this._onCancel},"I understand, continue without")))}_titleForPhase(e){switch(e){case v:return Object(d.a)("Secure your backup with a passphrase");case 1:return Object(d.a)("Confirm your passphrase");case 6:return Object(d.a)("Warning!");case 2:case 3:return Object(d.a)("Make a copy of your recovery key");case 4:return Object(d.a)("Starting backup...");case _:return Object(d.a)("Success!");default:return Object(d.a)("Create key backup")}}render(){const e=l.a("views.dialogs.BaseDialog");let t;if(this.state.error){const e=l.a("views.elements.DialogButtons");t=o.a.createElement("div",null,o.a.createElement("p",null,Object(d.a)("Unable to create key backup")),o.a.createElement("div",{className:"mx_Dialog_buttons"},o.a.createElement(e,{primaryButton:Object(d.a)("Retry"),onPrimaryButtonClick:this._createBackup,hasCancel:!0,onCancel:this._onCancel})))}else switch(this.state.phase){case v:t=this._renderPhasePassPhrase();break;case 1:t=this._renderPhasePassPhraseConfirm();break;case 2:t=this._renderPhaseShowKey();break;case 3:t=this._renderPhaseKeepItSafe();break;case 4:t=this._renderBusyPhase();break;case _:t=this._renderPhaseDone();break;case 6:t=this._renderPhaseOptOutConfirm()}return o.a.createElement(e,{className:"mx_CreateKeyBackupDialog",onFinished:this.props.onFinished,title:this._titleForPhase(this.state.phase),hasCancel:[v,_].includes(this.state.phase)},o.a.createElement("div",null,t))}}n()(f,"propTypes",{onFinished:p.a.func.isRequired})}}]);