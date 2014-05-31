Name:       system-popup
Summary:    system-popup application
Version:    0.1.31
Release:    1
Group:      Framework/system
License:    Apache License, Version 2.0
Source0:    system-popup-%{version}.tar.gz

Source1001:    org.tizen.poweroff-syspopup.manifest
Source1002:    org.tizen.lowmem-syspopup.manifest
Source1003:    org.tizen.lowbat-syspopup.manifest

%if "%{_repository}" == "wearable"
Source1004:    system-apps.manifest
Source1005:    org.tizen.crash-popup.manifest
%endif

%if "%{_repository}" == "mobile"
Source1006:    org.tizen.mmc-syspopup.manifest
Source1007:    org.tizen.usb-syspopup.manifest
Source1008:    org.tizen.usbotg-syspopup.manifest
%endif

BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(syspopup)
BuildRequires:  pkgconfig(syspopup-caller)
BuildRequires:  cmake
BuildRequires:  edje-bin
BuildRequires:  gettext-devel

%if "%{_repository}" == "wearable"
BuildRequires:  pkgconfig(deviced)
BuildRequires:  pkgconfig(feedback)
BuildRequires:  pkgconfig(efl-assist)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(capi-media-wav-player)
BuildRequires:  hash-signer
%endif

%if "%{_repository}" == "mobile"
BuildRequires:  pkgconfig(sysman)
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(sensor)
BuildRequires:  pkgconfig(devman_haptic)
BuildRequires:  pkgconfig(devman)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(pmapi)
BuildRequires:  pkgconfig(svi)
BuildRequires:  pkgconfig(appsvc)
%endif

%description
system-popup application


%package -n org.tizen.poweroff-syspopup
Summary:    system-popup application (poweroff popup,sysevent-alert)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.poweroff-syspopup
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

%if "%{_repository}" == "wearable"
%package -n org.tizen.crash-popup
Summary:    system popup application (crash system popup)
Group:      main
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.crash-popup
system popup application (crash system popup)
%endif

%if "%{_repository}" == "mobile"
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
%endif


%prep
%setup -q

export CFLAGS+=" -DTIZEN_ENGINEER_MODE"

%if 0%{?simulator}
%define simulator_state yes
%else
%define simulator_state no
%endif

%build

cp %{SOURCE1001} .
cp %{SOURCE1002} .
cp %{SOURCE1003} .

%if "%{_repository}" == "wearable"
cp %{SOURCE1004} .
cp %{SOURCE1005} .

cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DSYSTEM_APPS_MICRO=yes -DSIMULATOR=%{simulator_state}
%endif

%if "%{_repository}" == "mobile"
cp %{SOURCE1006} .
cp %{SOURCE1007} .
cp %{SOURCE1008} .

cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DSYSTEM_APPS_MICRO=no -DSIMULATOR=%{simulator_state}
%endif


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%if "%{_repository}" == "wearable"
%define tizen_sign 1
%define tizen_sign_base /usr/apps/org.tizen.poweroff-syspopup;/usr/apps/org.tizen.lowbat-syspopup;/usr/apps/org.tizen.lowmem-syspopup;/usr/apps/org.tizen.crash-popup
%define tizen_sign_level public
%define tizen_author_sign 1
%define tizen_dist_sign 1
%endif


mkdir -p %{buildroot}/usr/share/license

%if "%{_repository}" == "wearable"
%files
%manifest system-apps.manifest
%defattr(-,root,root,-)
%{_bindir}/sys_device_noti
%{_datadir}/license/system-popup
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%{_bindir}/popup-launcher
/usr/share/dbus-1/services/org.tizen.system.popup.service
/etc/smack/accesses2.d/system-apps.rule
%endif

%if "%{_repository}" == "mobile"
%files
%defattr(-,root,root,-)
%{_bindir}/sys_device_noti
%{_datadir}/license/system-popup
%{_datadir}/system-server/sys_device_noti/batt_full_icon.png
%{_datadir}/system-server/sys_device_noti/res/locale/*/LC_MESSAGES/*.mo
%endif

%files -n org.tizen.poweroff-syspopup
%manifest org.tizen.poweroff-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.poweroff-syspopup/bin/poweroff-popup
/usr/apps/org.tizen.poweroff-syspopup/res/edje/poweroff/poweroff.edj
/usr/share/packages/org.tizen.poweroff-syspopup.xml
/usr/share/license/org.tizen.poweroff-syspopup

%if "%{_repository}" == "wearable"
/usr/apps/org.tizen.poweroff-syspopup/author-signature.xml
/usr/apps/org.tizen.poweroff-syspopup/signature1.xml
/etc/smack/accesses2.d/org.tizen.poweroff-syspopup.rule
/usr/share/dbus-1/services/org.tizen.system.popup.poweroff.service
%endif

%if "%{_repository}" == "mobile"
/usr/apps/org.tizen.poweroff-syspopup/res/icon/org.tizen.poweroff-syspopup.png
/usr/share/process-info/poweroff-popup.ini
/opt/etc/smack/accesses.d/org.tizen.poweroff-syspopup.rule
/usr/apps/org.tizen.poweroff-syspopup/res/locale/*/LC_MESSAGES/*.mo
%endif

%files -n org.tizen.lowbat-syspopup
%manifest org.tizen.lowbat-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.lowbat-syspopup/bin/lowbatt-popup
/usr/share/packages/org.tizen.lowbat-syspopup.xml
/usr/share/license/org.tizen.lowbatt-syspopup

%if "%{_repository}" == "wearable"
/usr/apps/org.tizen.lowbat-syspopup/author-signature.xml
/usr/apps/org.tizen.lowbat-syspopup/signature1.xml
/etc/smack/accesses2.d/org.tizen.lowbat-syspopup.rule
%endif

%if "%{_repository}" == "mobile"
/usr/apps/org.tizen.lowbat-syspopup/res/edje/lowbatt/lowbatt.edj
/usr/apps/org.tizen.lowbat-syspopup/res/icon/org.tizen.lowbat-syspopup.png
/usr/share/process-info/lowbatt-popup.ini
/opt/etc/smack/accesses.d/org.tizen.lowbat-syspopup.rule
/usr/apps/org.tizen.lowbat-syspopup/res/locale/*/LC_MESSAGES/*.mo
%endif

%files -n org.tizen.lowmem-syspopup
%manifest org.tizen.lowmem-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.lowmem-syspopup/bin/lowmem-popup
/usr/share/packages/org.tizen.lowmem-syspopup.xml
/usr/share/license/org.tizen.lowmem-syspopup

%if "%{_repository}" == "wearable"
/usr/apps/org.tizen.lowmem-syspopup/author-signature.xml
/usr/apps/org.tizen.lowmem-syspopup/signature1.xml
/etc/smack/accesses2.d/org.tizen.lowmem-syspopup.rule
%endif

%if "%{_repository}" == "mobile"
/usr/apps/org.tizen.lowmem-syspopup/res/keysound/02_Warning.wav
/usr/apps/org.tizen.lowmem-syspopup/res/edje/lowmem/lowmem.edj
/usr/apps/org.tizen.lowmem-syspopup/res/icon/org.tizen.lowmem-syspopup.png
/usr/share/process-info/lowmem-popup.ini
/opt/etc/smack/accesses.d/org.tizen.lowmem-syspopup.rule
/usr/apps/org.tizen.lowmem-syspopup/res/locale/*/LC_MESSAGES/*.mo
%endif

%if "%{_repository}" == "wearable"
%files -n org.tizen.crash-popup
%manifest org.tizen.crash-popup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.crash-popup/bin/crash-popup
/usr/apps/org.tizen.crash-popup/author-signature.xml
/usr/apps/org.tizen.crash-popup/signature1.xml
/usr/share/packages/org.tizen.crash-popup.xml
/usr/share/license/org.tizen.crash-popup
/etc/smack/accesses2.d/org.tizen.crash-popup.rule
%endif

%if "%{_repository}" == "mobile"

%files -n org.tizen.mmc-syspopup
%manifest org.tizen.mmc-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.mmc-syspopup/bin/mmc-popup
/usr/share/packages/org.tizen.mmc-syspopup.xml
/usr/share/process-info/mmc-popup.ini
/usr/apps/org.tizen.mmc-syspopup/res/locale/*/LC_MESSAGES/*.mo
/opt/etc/smack/accesses.d/org.tizen.mmc-syspopup.rule
/usr/share/license/org.tizen.mmc-syspopup

%files -n org.tizen.usb-syspopup
%manifest org.tizen.usb-syspopup.manifest
%defattr(440,root,root,-)
%attr(555,app,app) /usr/apps/org.tizen.usb-syspopup/bin/usb-syspopup
%attr(440,app,app) /usr/apps/org.tizen.usb-syspopup/res/locale/*/LC_MESSAGES/usb-syspopup.mo
/usr/share/packages/org.tizen.usb-syspopup.xml
/opt/etc/smack/accesses.d/org.tizen.usb-syspopup.rule
/usr/share/license/org.tizen.usb-syspopup

%files -n org.tizen.usbotg-syspopup
%manifest org.tizen.usbotg-syspopup.manifest
%defattr(440,root,root,-)
%attr(555,app,app) /usr/apps/org.tizen.usbotg-syspopup/bin/usbotg-syspopup
/usr/share/packages/org.tizen.usbotg-syspopup.xml
/opt/etc/smack/accesses.d/org.tizen.usbotg-syspopup.rule
/usr/share/license/org.tizen.usbotg-syspopup

%endif
