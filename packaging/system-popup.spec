
%define PROFILE none
#Main applications
%define poweroff_popup off
%define crash_popup off
%define system_popup off
%define signal_sender off
%define system_servant off
%define notification_service off
%define usbhost_list_app off

#sub-popups of system-popup
%define battery_popup off
%define cooldown_popup off
%define datausage_popup off
%define mmc_popup off
%define ode_popup off
%define recovery_popup off
%define storage_popup off
%define usb_popup off
%define watchdog_popup off

%if "%{?tizen_profile_name}" == "mobile"
%define PROFILE mobile
#Main applicaitons
%define poweroff_popup on
%define crash_popup on
%define system_popup on
%define signal_sender on
%define system_servant on
%define notification_service on
%define usbhost_list_app on
#sub-popups of system-popup
%define battery_popup on
%define cooldown_popup on
%define datausage_popup on
%define mmc_popup on
%define storage_popup on
%define usb_popup on
%define watchdog_popup on
%endif

%if "%{?tizen_profile_name}" == "wearable"
%define PROFILE wearable
#Main applicaitons
%define poweroff_popup on
%define crash_popup on
%define system_popup on
%define system_servant on
#sub-popups of system-popup
%define battery_popup on
%define cooldown_popup on
%define storage_popup on
%define watchdog_popup on
%endif

%if "%{?tizen_profile_name}" == "tv"
%define PROFILE tv
#Main applications
%define crash_popup on
%endif

Name:       system-popup
Summary:    system-popup application
Version:    0.1.31
Release:    1
Group:      Framework/system
License:    Apache-2.0
Source0:    system-popup-%{version}.tar.gz
Source1:    system-apps.manifest
Source1001:    org.tizen.poweroff-syspopup.manifest
Source1015:    org.tizen.crash-popup.manifest
Source2001:    org.tizen.system-syspopup.manifest
Source2003:    org.tizen.system-signal-sender.manifest
Source2005:    org.tizen.host-devices.manifest

BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(deviced)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(syspopup)
BuildRequires:  pkgconfig(syspopup-caller)
BuildRequires:  pkgconfig(feedback)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(capi-media-wav-player)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(tts)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  cmake
BuildRequires:  edje-bin
BuildRequires:  gettext-devel

Requires(post): /usr/bin/vconftool

%description
system-popup application (poweroff popup,sysevent-alert).

%if %{?crash_popup} == on
%package -n org.tizen.crash-popup
Summary:    system popup application (crash system popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.crash-popup
system popup application (crash system popup)
%endif

%if %{?poweroff_popup} == on
%package -n org.tizen.poweroff-syspopup
Summary:    poweroff-popup application
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.poweroff-syspopup
poweroff-popup application.
%endif

%if %{?signal_sender} == on
%package -n org.tizen.system-signal-sender
Summary:    system FW signal sender
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.system-signal-sender
system FW signal sender
%endif

%if %{?usbhost_list_app} == on
%package -n org.tizen.host-devices
Summary:    Show usb host device list
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.host-devices
Show usb host device list
%endif



%if %{?system_popup} == on
%package -n org.tizen.system-syspopup
Summary:    system popup application
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.system-syspopup
system popup application

%if %{?battery_popup} == on
%package -n org.tizen.lowbat-syspopup
Summary:    system-popup application (lowbat popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.lowbat-syspopup
system-popup application (lowbat popup).
%endif

%if %{?storage_popup} == on
%package -n org.tizen.lowmem-syspopup
Summary:    system-popup application (lowmem popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.lowmem-syspopup
system-popup application (lowmem popup).
%endif

%if %{?mmc_popup} == on
%package -n org.tizen.mmc-syspopup
Summary:    system-popup application (mmc  popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.mmc-syspopup
system-popup application (mmc  popup).
%endif

%if %{?usb_popup} == on
%package -n org.tizen.usb-syspopup
Summary:    system-popup application (usb popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.usb-syspopup
system-popup application (usb popup).

%package -n org.tizen.usbotg-syspopup
Summary:    system-popup application (usb otg popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.usbotg-syspopup
system-popup application (usb otg popup).
%endif

%if %{?datausage_popup} == on
%package -n org.tizen.datausage-syspopup
Summary:    system popup application (data usage popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.datausage-syspopup
system popup application (data usage popup)
%endif

%endif # system_popup

%prep
%setup -q

%if 0%{?simulator}
%define simulator_state yes
%else
%define simulator_state no
%endif

%build

cp %{SOURCE1} .

%if %{poweroff_popup} == on
cp %{SOURCE1001} .
%endif

%if %{crash_popup} == on
cp %{SOURCE1015} .
%endif

%if %{system_popup} == on
cp %{SOURCE2001} .
%endif

%if %{signal_sender} == on
cp %{SOURCE2003} .
%endif

%if %{usbhost_list_app} == on
cp %{SOURCE2005} .
%endif

cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DSIMULATOR=%{simulator_state} \
		-DPROFILE=%{PROFILE} \
		-DPOWEROFF_POPUP=%{poweroff_popup} \
		-DCRASH_POPUP=%{crash_popup} \
		-DSYSTEM_POPUP=%{system_popup} \
		-DSIGNAL_SENDER=%{signal_sender} \
		-DSYSTEM_SERVANT=%{system_servant} \
		-DNOTIFICATION_SERVICE=%{notification_service} \
		-DUSBHOST_LIST_APP=%{usbhost_list_app} \
		-DBATTERY_POPUP=%{battery_popup} \
		-DCOOLDOWN_POPUP=%{cooldown_popup} \
		-DDATAUSAGE_POPUP=%{datausage_popup} \
		-DMMC_POPUP=%{mmc_popup} \
		-DODE_POPUP=%{ode_popup} \
		-DRECOVERY_POPUP=%{recovery_popup} \
		-DSTORAGE_POPUP=%{storage_popup} \
		-DUSB_POPUP=%{usb_popup} \
		-DWATCHDOG_POPUP=%{watchdog_popup}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/usr/share/license


%files
%manifest system-apps.manifest
%defattr(-,system,system,-)
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%{_bindir}/popup-launcher
%{_datadir}/license/system-popup
/usr/share/dbus-1/system-services/org.tizen.system.popup.service
/etc/smack/accesses.d/system-apps.efl

%if %{notification_service} == on
%{_datadir}/system-apps/res/icons/batt_full_icon.png
%{_datadir}/system-apps/res/icons/batt_full_indicator.png
/usr/share/system-apps/res/icons/datausage_warning.png
/usr/share/system-apps/res/icons/led_torch.png
/usr/share/system-apps/res/icons/sdcard_encryption.png
/usr/share/system-apps/res/icons/sdcard_decryption.png
/usr/share/system-apps/res/icons/sdcard_encryption_error.png
/usr/share/system-apps/res/icons/sdcard_decryption_error.png
/usr/share/system-apps/res/icons/tima.png
/usr/share/system-apps/res/icons/usb.png
%endif

%if %{system_servant} == on
%{_bindir}/system-servant
%endif

%if %{crash_popup} == on
%files -n org.tizen.crash-popup
%manifest org.tizen.crash-popup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.crash-popup/bin/crash-popup
/usr/share/packages/org.tizen.crash-popup.xml
/usr/share/license/org.tizen.crash-popup
/etc/smack/accesses.d/org.tizen.crash-popup.efl
%endif

%if %{system_popup} == on
%files -n org.tizen.system-syspopup
%manifest org.tizen.system-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.system-syspopup/bin/system-syspopup
/usr/share/packages/org.tizen.system-syspopup.xml
/usr/share/license/org.tizen.system-syspopup
/etc/smack/accesses.d/org.tizen.system-syspopup.efl

%if %{battery_popup} == on
%files -n org.tizen.lowbat-syspopup
%endif
%if %{storage_popup} == on
%files -n org.tizen.lowmem-syspopup
%endif
%if %{mmc_popup} == on
%files -n org.tizen.mmc-syspopup
%endif
%if %{usb_popup} == on
%files -n org.tizen.usb-syspopup
%files -n org.tizen.usbotg-syspopup
%endif
%if %{datausage_popup} == on
%files -n org.tizen.datausage-syspopup
%endif

%endif # system_popup

%if %{poweroff_popup} == on
%files -n org.tizen.poweroff-syspopup
%manifest org.tizen.poweroff-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.poweroff-syspopup/bin/poweroff-popup
/usr/share/packages/org.tizen.poweroff-syspopup.xml
/usr/share/license/org.tizen.poweroff-syspopup
/etc/smack/accesses.d/org.tizen.poweroff-syspopup.efl
%endif

%if %{signal_sender} == on
%files -n org.tizen.system-signal-sender
%manifest org.tizen.system-signal-sender.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.system-signal-sender/bin/system-signal-sender
/usr/share/packages/org.tizen.system-signal-sender.xml
/usr/share/license/org.tizen.system-signal-sender
/etc/smack/accesses.d/org.tizen.system-signal-sender.efl
%endif

%if %{usbhost_list_app} == on
%files -n org.tizen.host-devices
%manifest org.tizen.host-devices.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.host-devices/bin/org.tizen.host-devices
/usr/apps/org.tizen.host-devices/res/edje/host-devices/host-devices.edj
/usr/share/packages/org.tizen.host-devices.xml
/usr/share/license/org.tizen.host-devices
/etc/smack/accesses.d/org.tizen.host-devices.efl
%endif
