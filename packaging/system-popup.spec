#sbs-git:slp/pkgs/s/system-popup system-popup 0.1.7 c9f24c4ffe1f2306a5ad3ffef4074e6f65bff6a3
Name:       system-popup
Summary:    system-popup application (poweroff popup,sysevent-alert)
Version:    0.1.31
Release:    1
VCS:        magnolia/framework/system/system-popup#submit/master/20130425.080357-328-g0cfacbaf9ebfb672f3c4b76263635c18e19a3b67
Group:      Framework/system
License:    Apache-2.0
Source0:    system-popup-%{version}.tar.gz
Source1:    system-apps.manifest
Source1001:    org.tizen.poweroff-syspopup.manifest
Source1003:    org.tizen.lowmem-syspopup.manifest
Source1005:    org.tizen.lowbat-syspopup.manifest
Source1015:    org.tizen.crash-popup.manifest
Source1007:    org.tizen.mmc-syspopup.manifest
Source1008:    org.tizen.mmc-syspopup.efl
Source1009:    org.tizen.usb-syspopup.manifest
Source1010:    org.tizen.usb-syspopup.efl
Source1011:    org.tizen.usbotg-syspopup.manifest
Source1012:    org.tizen.usbotg-syspopup.efl
Source1013:    org.tizen.datausage-syspopup.manifest
Source1014:    org.tizen.datausage-syspopup.efl
Source2001:    org.tizen.system-syspopup.manifest
Source2002:    org.tizen.system-syspopup.efl
Source2003:    org.tizen.system-signal-sender.manifest
Source2004:    org.tizen.system-signal-sender.efl
Source2005:    org.tizen.host-devices.manifest
Source2006:    org.tizen.host-devices.efl

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
BuildRequires:  pkgconfig(efl-assist)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(capi-media-wav-player)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(tts)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  model-build-features
BuildRequires:  cmake
BuildRequires:  edje-bin
BuildRequires:  gettext-devel

Requires(post): /usr/bin/vconftool

%description
system-popup application (poweroff popup,sysevent-alert).

%package -n org.tizen.lowbat-syspopup
Summary:    system-popup application (lowbat popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.lowbat-syspopup
system-popup application (lowbat popup).

%package -n org.tizen.lowmem-syspopup
Summary:    system-popup application (lowmem popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.lowmem-syspopup
system-popup application (lowmem popup).

%package -n org.tizen.crash-popup
Summary:    system popup application (crash system popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.crash-popup
system popup application (crash system popup)

%package -n org.tizen.system-syspopup
Summary:    system popup application
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.system-syspopup
system popup application

%if "%{?tizen_profile_name}" == "mobile"

%package -n org.tizen.poweroff-syspopup
Summary:    poweroff-popup application
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.poweroff-syspopup
poweroff-popup application.

%package -n org.tizen.mmc-syspopup
Summary:    system-popup application (mmc  popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.mmc-syspopup
system-popup application (mmc  popup).

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

%package -n org.tizen.datausage-syspopup
Summary:    system popup application (data usage popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.datausage-syspopup
system popup application (data usage popup)

%package -n org.tizen.system-signal-sender
Summary:    system FW signal sender
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.system-signal-sender
system FW signal sender

%package -n org.tizen.host-devices
Summary:    Show usb host device list
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.host-devices
Show usb host device list

%endif

%prep
%setup -q

%if 0%{?simulator}
%define simulator_state yes
%else
%define simulator_state no
%endif

%build

%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS+=" -DTIZEN_ENGINEER_MODE"
%endif

%if "%{?tizen_profile_name}" == "wearable"
export CFLAGS+=" -DSYSTEM_APPS_MICRO"
export CFLAGS+=" -DSYSTEM_APPS_MICRO_3"
%endif

%if "%{?tizen_profile_name}" == "mobile"
export CFLAGS+=" -DSYSTEM_APPS_LITE"
%endif

%if "%{?model_build_feature_formfactor}" == "rectangle"
export CFLAGS+=" -DSYSTEM_APPS_RECTANGLE"
%endif

%if "%{?model_build_feature_formfactor}" == "circle"
export CFLAGS+=" -DSYSTEM_APPS_CIRCLE"
%endif


cp %{SOURCE1} .
cp %{SOURCE1003} .
cp %{SOURCE1005} .
cp %{SOURCE1015} .
cp %{SOURCE2001} .


%if "%{?tizen_profile_name}" == "wearable"
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DSYSTEM_APPS_MICRO=yes -DSIMULATOR=%{simulator_state}
%endif

%if "%{?tizen_profile_name}" == "mobile"
cp %{SOURCE1001} .
cp %{SOURCE1007} .
cp %{SOURCE1008} .
cp %{SOURCE1009} .
cp %{SOURCE1010} .
cp %{SOURCE1011} .
cp %{SOURCE1012} .
cp %{SOURCE1013} .
cp %{SOURCE1014} .
cp %{SOURCE2002} .
cp %{SOURCE2003} .
cp %{SOURCE2004} .
cp %{SOURCE2005} .
cp %{SOURCE2006} .

cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DSYSTEM_APPS_MICRO=no  -DSIMULATOR=%{simulator_state}
%endif

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install


mkdir -p %{buildroot}/usr/share/license

%if "%{?tizen_profile_name}" == "wearable"
%post
vconftool set -t bool db/private/mobiledata/on_popup/check 0 -u 5000 -s system::vconf_system
vconftool set -t bool db/private/mobiledata/off_popup/check 0 -u 5000 -s system::vconf_system
%endif


%files
%manifest system-apps.manifest
%defattr(-,root,root,-)
%if "%{?tizen_profile_name}" == "mobile"
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
%{_bindir}/systemfw-app-test
%endif
%{_bindir}/sys_device_noti
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%{_bindir}/popup-launcher
%{_bindir}/system-servant
%{_datadir}/license/system-popup
/usr/share/dbus-1/services/org.tizen.system.popup.service
/etc/smack/accesses.d/system-apps.efl

%files -n org.tizen.lowbat-syspopup
%manifest org.tizen.lowbat-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.lowbat-syspopup/bin/lowbatt-popup
/usr/share/packages/org.tizen.lowbat-syspopup.xml
/usr/share/license/org.tizen.lowbatt-syspopup
/etc/smack/accesses.d/org.tizen.lowbat-syspopup.efl
/usr/apps/org.tizen.lowbat-syspopup/res/edje/lowbatt/lowbatt.edj
%if "%{?tizen_profile_name}" == "wearable"
/usr/apps/org.tizen.lowbat-syspopup/res/table/system-color.xml
/usr/apps/org.tizen.lowbat-syspopup/res/table/system-font.xml
%endif

%files -n org.tizen.lowmem-syspopup
%manifest org.tizen.lowmem-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.lowmem-syspopup/bin/lowmem-popup
/usr/share/packages/org.tizen.lowmem-syspopup.xml
/usr/share/license/org.tizen.lowmem-syspopup
/etc/smack/accesses.d/org.tizen.lowmem-syspopup.efl
/usr/apps/org.tizen.lowmem-syspopup/res/edje/lowmem/lowmem.edj
%if "%{?tizen_profile_name}" == "wearable"
/usr/apps/org.tizen.lowmem-syspopup/res/table/system-color.xml
/usr/apps/org.tizen.lowmem-syspopup/res/table/system-font.xml
%endif

%files -n org.tizen.crash-popup
%manifest org.tizen.crash-popup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.crash-popup/bin/crash-popup
/usr/share/packages/org.tizen.crash-popup.xml
/usr/share/license/org.tizen.crash-popup
/etc/smack/accesses.d/org.tizen.crash-popup.efl
%if "%{?tizen_profile_name}" == "wearable"
/usr/apps/org.tizen.crash-popup/res/table/system-color.xml
/usr/apps/org.tizen.crash-popup/res/table/system-font.xml
%endif
/usr/apps/org.tizen.crash-popup/res/edje/crash/crash.edj

%files -n org.tizen.system-syspopup
%manifest org.tizen.system-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.system-syspopup/bin/system-syspopup
/usr/apps/org.tizen.system-syspopup/res/edje/system/system.edj
/usr/share/packages/org.tizen.system-syspopup.xml
/usr/share/license/org.tizen.system-syspopup
/etc/smack/accesses.d/org.tizen.system-syspopup.efl
%if "%{?tizen_profile_name}" == "wearable"
/usr/apps/org.tizen.system-syspopup/res/table/system-color.xml
/usr/apps/org.tizen.system-syspopup/res/table/system-font.xml
%endif

%if "%{?tizen_profile_name}" == "mobile"
%files -n org.tizen.poweroff-syspopup
%manifest org.tizen.poweroff-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.poweroff-syspopup/bin/poweroff-popup
/usr/share/packages/org.tizen.poweroff-syspopup.xml
/usr/share/license/org.tizen.poweroff-syspopup
/etc/smack/accesses.d/org.tizen.poweroff-syspopup.efl

%files -n org.tizen.mmc-syspopup
%manifest org.tizen.mmc-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.mmc-syspopup/bin/mmc-popup
/usr/share/packages/org.tizen.mmc-syspopup.xml
/usr/share/license/org.tizen.mmc-syspopup
/etc/smack/accesses.d/org.tizen.mmc-syspopup.efl

%files -n org.tizen.usb-syspopup
%manifest org.tizen.usb-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.usb-syspopup/bin/usb-syspopup
/usr/share/packages/org.tizen.usb-syspopup.xml
/usr/share/license/org.tizen.usb-syspopup
/usr/apps/org.tizen.usb-syspopup/res/icons/usb_icon.png
/etc/smack/accesses.d/org.tizen.usb-syspopup.efl

%files -n org.tizen.usbotg-syspopup
%manifest org.tizen.usbotg-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.usbotg-syspopup/bin/usbotg-syspopup
/usr/share/packages/org.tizen.usbotg-syspopup.xml
/usr/share/license/org.tizen.usbotg-syspopup
/usr/apps/org.tizen.usbotg-syspopup/res/icons/usb_icon.png
/etc/smack/accesses.d/org.tizen.usbotg-syspopup.efl

%files -n org.tizen.datausage-syspopup
%manifest org.tizen.datausage-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.datausage-syspopup/bin/datausage-popup
/usr/share/packages/org.tizen.datausage-syspopup.xml
/usr/share/license/org.tizen.datausage-syspopup
/etc/smack/accesses.d/org.tizen.datausage-syspopup.efl

%files -n org.tizen.system-signal-sender
%manifest org.tizen.system-signal-sender.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.system-signal-sender/bin/system-signal-sender
/usr/share/packages/org.tizen.system-signal-sender.xml
/usr/share/license/org.tizen.system-signal-sender
/etc/smack/accesses.d/org.tizen.system-signal-sender.efl

%files -n org.tizen.host-devices
%manifest org.tizen.host-devices.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.host-devices/bin/org.tizen.host-devices
/usr/apps/org.tizen.host-devices/res/edje/host-devices/host-devices.edj
/usr/share/packages/org.tizen.host-devices.xml
/usr/share/license/org.tizen.host-devices
/etc/smack/accesses.d/org.tizen.host-devices.efl

%endif
