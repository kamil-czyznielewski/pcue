# Generated by the VisualDSP++ IDDE

# Note:  Any changes made to this Makefile will be lost the next time the
# matching project file is loaded into the IDDE.  If you wish to preserve
# changes, rename this file and run it externally to the IDDE.

# The syntax of this Makefile is such that GNU Make v3.77 or higher is
# required.

# The current working directory should be the directory in which this
# Makefile resides.

# Supported targets:
#     inetd-BF537_Debug
#     inetd-BF537_Debug_clean

# Define this variable if you wish to run this Makefile on a host
# other than the host that created it and VisualDSP++ may be installed
# in a different directory.

ADI_DSP=C:\Program Files\Analog Devices\VisualDSP 5.0


# $VDSP is a gmake-friendly version of ADI_DIR

empty:=
space:= $(empty) $(empty)
VDSP_INTERMEDIATE=$(subst \,/,$(ADI_DSP))
VDSP=$(subst $(space),\$(space),$(VDSP_INTERMEDIATE))

RM=cmd /C del /F /Q

#
# Begin "inetd-BF537_Debug" configuration
#

ifeq ($(MAKECMDGOALS),inetd-BF537_Debug)

inetd-BF537_Debug : ./Debug/inetd-BF537.dxe 

Debug/CharGen_Server_ThreadType.doj :CharGen_Server_ThreadType.c CharGen_Server_ThreadType.h VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h charcb.h $(VDSP)/Blackfin/include/lwip/sockets.h $(VDSP)/Blackfin/include/lwip/ip_addr.h $(VDSP)/Blackfin/include/lwip/arch.h $(VDSP)/Blackfin/include/lwip/cc.h $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/stdio.h $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/lwip/inet.h $(VDSP)/Blackfin/include/stddef.h IPv4_util.h 
	@echo ".\CharGen_Server_ThreadType.c"
	$(VDSP)/ccblkfn.exe -c .\CharGen_Server_ThreadType.c -file-attr ProjectName=inetd-BF537 -O -Ov100 -g -structs-do-not-overlap -no-multiline -double-size-32 -decls-strong -flags-compiler --diag_warning,implicit_func_decl -warn-protos -threads -si-revision 0.2 -proc ADSP-BF537 -o .\Debug\CharGen_Server_ThreadType.doj -MM

Debug/CharGen_Worker_ThreadType.doj :CharGen_Worker_ThreadType.c CharGen_Worker_ThreadType.h VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h charcb.h $(VDSP)/Blackfin/include/lwip/sockets.h $(VDSP)/Blackfin/include/lwip/ip_addr.h $(VDSP)/Blackfin/include/lwip/arch.h $(VDSP)/Blackfin/include/lwip/cc.h $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/stdio.h $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/lwip/inet.h $(VDSP)/Blackfin/include/stddef.h IPv4_util.h 
	@echo ".\CharGen_Worker_ThreadType.c"
	$(VDSP)/ccblkfn.exe -c .\CharGen_Worker_ThreadType.c -file-attr ProjectName=inetd-BF537 -O -Ov100 -g -structs-do-not-overlap -no-multiline -double-size-32 -decls-strong -flags-compiler --diag_warning,implicit_func_decl -warn-protos -threads -si-revision 0.2 -proc ADSP-BF537 -o .\Debug\CharGen_Worker_ThreadType.doj -MM

Debug/Discard_Server_ThreadType.doj :Discard_Server_ThreadType.c Discard_Server_ThreadType.h VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h charcb.h $(VDSP)/Blackfin/include/lwip/sockets.h $(VDSP)/Blackfin/include/lwip/ip_addr.h $(VDSP)/Blackfin/include/lwip/arch.h $(VDSP)/Blackfin/include/lwip/cc.h $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/stdio.h $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/lwip/inet.h $(VDSP)/Blackfin/include/stddef.h IPv4_util.h 
	@echo ".\Discard_Server_ThreadType.c"
	$(VDSP)/ccblkfn.exe -c .\Discard_Server_ThreadType.c -file-attr ProjectName=inetd-BF537 -O -Ov100 -g -structs-do-not-overlap -no-multiline -double-size-32 -decls-strong -flags-compiler --diag_warning,implicit_func_decl -warn-protos -threads -si-revision 0.2 -proc ADSP-BF537 -o .\Debug\Discard_Server_ThreadType.doj -MM

Debug/Discard_Worker_ThreadType.doj :Discard_Worker_ThreadType.c Discard_Worker_ThreadType.h VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h charcb.h $(VDSP)/Blackfin/include/lwip/sockets.h $(VDSP)/Blackfin/include/lwip/ip_addr.h $(VDSP)/Blackfin/include/lwip/arch.h $(VDSP)/Blackfin/include/lwip/cc.h $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/stdio.h $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/lwip/inet.h $(VDSP)/Blackfin/include/stddef.h IPv4_util.h 
	@echo ".\Discard_Worker_ThreadType.c"
	$(VDSP)/ccblkfn.exe -c .\Discard_Worker_ThreadType.c -file-attr ProjectName=inetd-BF537 -O -Ov100 -g -structs-do-not-overlap -no-multiline -double-size-32 -decls-strong -flags-compiler --diag_warning,implicit_func_decl -warn-protos -threads -si-revision 0.2 -proc ADSP-BF537 -o .\Debug\Discard_Worker_ThreadType.doj -MM

Debug/Echo_Server_ThreadType.doj :Echo_Server_ThreadType.c Echo_Server_ThreadType.h VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h charcb.h $(VDSP)/Blackfin/include/lwip/sockets.h $(VDSP)/Blackfin/include/lwip/ip_addr.h $(VDSP)/Blackfin/include/lwip/arch.h $(VDSP)/Blackfin/include/lwip/cc.h $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/stdio.h $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/lwip/inet.h $(VDSP)/Blackfin/include/stddef.h IPv4_util.h 
	@echo ".\Echo_Server_ThreadType.c"
	$(VDSP)/ccblkfn.exe -c .\Echo_Server_ThreadType.c -file-attr ProjectName=inetd-BF537 -O -Ov100 -g -structs-do-not-overlap -no-multiline -double-size-32 -decls-strong -flags-compiler --diag_warning,implicit_func_decl -warn-protos -threads -si-revision 0.2 -proc ADSP-BF537 -o .\Debug\Echo_Server_ThreadType.doj -MM

Debug/Echo_Worker_ThreadType.doj :Echo_Worker_ThreadType.c Echo_Worker_ThreadType.h VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h charcb.h $(VDSP)/Blackfin/include/lwip/sockets.h $(VDSP)/Blackfin/include/lwip/ip_addr.h $(VDSP)/Blackfin/include/lwip/arch.h $(VDSP)/Blackfin/include/lwip/cc.h $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/stdio.h $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/lwip/inet.h $(VDSP)/Blackfin/include/stddef.h IPv4_util.h $(VDSP)/Blackfin/include/cdefBF537.h $(VDSP)/Blackfin/include/cdefBF534.h $(VDSP)/Blackfin/include/cdef_LPBlackfin.h 
	@echo ".\Echo_Worker_ThreadType.c"
	$(VDSP)/ccblkfn.exe -c .\Echo_Worker_ThreadType.c -file-attr ProjectName=inetd-BF537 -O -Ov100 -g -structs-do-not-overlap -no-multiline -double-size-32 -decls-strong -flags-compiler --diag_warning,implicit_func_decl -warn-protos -threads -si-revision 0.2 -proc ADSP-BF537 -o .\Debug\Echo_Worker_ThreadType.doj -MM

./Debug/ExceptionHandler-BF537.doj :./ExceptionHandler-BF537.asm ./VDK.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h 
	@echo ".\ExceptionHandler-BF537.asm"
	$(VDSP)/easmblkfn.exe .\ExceptionHandler-BF537.asm -proc ADSP-BF537 -file-attr ProjectName=inetd-BF537 -g -si-revision 0.2 -o .\Debug\ExceptionHandler-BF537.doj -MM

Debug/IPv4_util.doj :IPv4_util.c $(VDSP)/Blackfin/include/stdio.h $(VDSP)/Blackfin/include/yvals.h 
	@echo ".\IPv4_util.c"
	$(VDSP)/ccblkfn.exe -c .\IPv4_util.c -file-attr ProjectName=inetd-BF537 -O -Ov100 -g -structs-do-not-overlap -no-multiline -double-size-32 -decls-strong -flags-compiler --diag_warning,implicit_func_decl -warn-protos -threads -si-revision 0.2 -proc ADSP-BF537 -o .\Debug\IPv4_util.doj -MM

Debug/lwip_sysboot_threadtype.doj :lwip_sysboot_threadtype.c lwip_sysboot_threadtype.h VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h $(VDSP)/Blackfin/include/lwip/cglobals.h $(VDSP)/Blackfin/include/sys/exception.h $(VDSP)/Blackfin/include/services/services.h $(VDSP)/Blackfin/include/services_types.h $(VDSP)/Blackfin/include/stdbool.h $(VDSP)/Blackfin/include/stddef.h $(VDSP)/Blackfin/include/cdefBF537.h $(VDSP)/Blackfin/include/cdefBF534.h $(VDSP)/Blackfin/include/cdef_LPBlackfin.h $(VDSP)/Blackfin/include/ccblkfn.h $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/builtins.h $(VDSP)/Blackfin/include/sys/builtins_support.h $(VDSP)/Blackfin/include/fract_typedef.h $(VDSP)/Blackfin/include/fr2x16_typedef.h $(VDSP)/Blackfin/include/r2x16_typedef.h $(VDSP)/Blackfin/include/raw_typedef.h $(VDSP)/Blackfin/include/sys/anomaly_macros_rtl.h $(VDSP)/Blackfin/include/sys/mc_typedef.h $(VDSP)/Blackfin/include/services/int/adi_int.h $(VDSP)/Blackfin/include/services/dcb/adi_dcb.h $(VDSP)/Blackfin/include/services/dma/adi_dma.h $(VDSP)/Blackfin/include/services/ebiu/adi_ebiu.h $(VDSP)/Blackfin/include/services/rtc/adi_rtc.h $(VDSP)/Blackfin/include/time.h $(VDSP)/Blackfin/include/xcycle_count.h $(VDSP)/Blackfin/include/cycle_count_bf.h $(VDSP)/Blackfin/include/services/pwr/adi_pwr.h $(VDSP)/Blackfin/include/services/tmr/adi_tmr.h $(VDSP)/Blackfin/include/services/flag/adi_flag.h $(VDSP)/Blackfin/include/services/ports/adi_ports.h $(VDSP)/Blackfin/include/services/ports/adi_ports_bf534.h $(VDSP)/Blackfin/include/services/sem/adi_sem.h $(VDSP)/Blackfin/include/drivers/adi_dev.h $(VDSP)/Blackfin/include/lwip/kernel_abs.h $(VDSP)/Blackfin/include/lwip/ADI_TOOLS_IOEThreadType.h $(VDSP)/Blackfin/include/lwip/sockets.h $(VDSP)/Blackfin/include/lwip/ip_addr.h $(VDSP)/Blackfin/include/lwip/arch.h $(VDSP)/Blackfin/include/lwip/cc.h $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/stdio.h $(VDSP)/Blackfin/include/lwip/inet.h $(VDSP)/Blackfin/include/lwip/ADI_ETHER_BF537.h $(VDSP)/Blackfin/include/drivers/ethernet/ADI_ETHER_BF537.H $(VDSP)/Blackfin/include/drivers/ethernet/ADI_ETHER.h 
	@echo ".\lwip_sysboot_threadtype.c"
	$(VDSP)/ccblkfn.exe -c .\lwip_sysboot_threadtype.c -file-attr ProjectName=inetd-BF537 -O -Ov100 -g -structs-do-not-overlap -no-multiline -double-size-32 -decls-strong -flags-compiler --diag_warning,implicit_func_decl -warn-protos -threads -si-revision 0.2 -proc ADSP-BF537 -o .\Debug\lwip_sysboot_threadtype.doj -MM

VDK.h VDK.cpp tcpip_template.rbld :$(VDSP)/blackfin/vdk/VDK.cpp.tf $(VDSP)/blackfin/vdk/VDK.h.tf $(VDSP)/blackfin/vdk/VDKGen.exe ./tcpip_template.vdk tcpip_template.rbld 
	@echo ".\tcpip_template.vdk"
	$(VDSP)/Blackfin\vdk\vdkgen.exe .\tcpip_template.vdk -proc ADSP-BF537 -si-revision 0.2 -MM

Debug/VDK.doj :VDK.cpp VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h $(VDSP)/Blackfin/include/stddef.h $(VDSP)/Blackfin/include/assert.h $(VDSP)/Blackfin/include/sys/platform.h $(VDSP)/Blackfin/include/sys/_adi_platform.h $(VDSP)/Blackfin/include/cdefBF537.h $(VDSP)/Blackfin/include/cdefBF534.h $(VDSP)/Blackfin/include/cdef_LPBlackfin.h $(VDSP)/Blackfin/include/cplus/new $(VDSP)/Blackfin/include/cplus/exception $(VDSP)/Blackfin/include/cplus/xstddef $(VDSP)/Blackfin/include/xsyslock.h $(VDSP)/Blackfin/include/sys/adi_rtl_sync.h $(VDSP)/Blackfin/include/stdbool.h $(VDSP)/Blackfin/include/stdint.h $(VDSP)/Blackfin/include/cplus/cstdlib $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/cplus/cstddef $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/VDK_Internals.h $(VDSP)/Blackfin/include/sys/exception.h lwip_sysboot_threadtype.h $(VDSP)/Blackfin/include/lwip/ADI_TOOLS_IOEThreadType.h CharGen_Server_ThreadType.h CharGen_Worker_ThreadType.h Discard_Server_ThreadType.h Discard_Worker_ThreadType.h Echo_Server_ThreadType.h Echo_Worker_ThreadType.h 
	@echo ".\VDK.cpp"
	$(VDSP)/ccblkfn.exe -c .\VDK.cpp -c++ -file-attr ProjectName=inetd-BF537 -O -Ov100 -g -structs-do-not-overlap -ignore-std -no-multiline -double-size-32 -decls-strong -flags-compiler --diag_warning,implicit_func_decl -warn-protos -threads -si-revision 0.2 -proc ADSP-BF537 -o .\Debug\VDK.doj -MM

./Debug/inetd-BF537.dxe :./VDK-BF537.ldf ./VDK.h $(VDSP)/Blackfin/lib/bf534_rev_0.0/TMK-BF532.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/vdk-CORE-BF532.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/vdk-i-BF532.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libsmall532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/__initsbsz532.doj $(VDSP)/Blackfin/lib/bf534_rev_0.0/libio532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libc532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/librt532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libevent532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libcpp532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libcpprt532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libx532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libf64ieee532y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libdsp532y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libsftflt532y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/libetsi532y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/Debug/libssl537_vdky.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/Debug/libdrv537y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/idle532mty.doj $(VDSP)/Blackfin/lib/bf534_rev_0.0/librt_fileio532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.0/crtsfc532mty.doj $(VDSP)/BLACKFIN/lib/ADI_ETHER_BF537.dlb $(VDSP)/BLACKFIN/lib/Kervdkbf537.dlb $(VDSP)/BLACKFIN/lib/liblwIPbf537.dlb $(VDSP)/BLACKFIN/lib/Tcpipbf537.dlb ./Debug/CharGen_Server_ThreadType.doj ./Debug/CharGen_Worker_ThreadType.doj ./Debug/Discard_Server_ThreadType.doj ./Debug/Discard_Worker_ThreadType.doj ./Debug/Echo_Server_ThreadType.doj ./Debug/Echo_Worker_ThreadType.doj ./Debug/ExceptionHandler-BF537.doj ./Debug/IPv4_util.doj ./Debug/lwip_sysboot_threadtype.doj ./Debug/VDK.doj $(VDSP)/BLACKFIN/lib/cplbtab537.doj $(VDSP)/Blackfin/lib/bf534_rev_0.0/crtn532mty.doj 
	@echo "Linking..."
	$(VDSP)/ccblkfn.exe .\Debug\CharGen_Server_ThreadType.doj .\Debug\CharGen_Worker_ThreadType.doj .\Debug\Discard_Server_ThreadType.doj .\Debug\Discard_Worker_ThreadType.doj .\Debug\Echo_Server_ThreadType.doj .\Debug\Echo_Worker_ThreadType.doj .\Debug\ExceptionHandler-BF537.doj .\Debug\IPv4_util.doj .\Debug\lwip_sysboot_threadtype.doj .\Debug\VDK.doj -T .\VDK-BF537.ldf -L .\Debug -flags-link -MDUSE_SDRAM_HEAP -add-debug-libpaths -flags-link -od,.\Debug -o .\Debug\inetd-BF537.dxe -proc ADSP-BF537 -flags-link ADI_ETHER_BF537.dlb,Kervdkbf537.dlb,liblwIPbf537.dlb,Tcpipbf537.dlb -si-revision 0.2 -flags-link -MM

endif

ifeq ($(MAKECMDGOALS),inetd-BF537_Debug_clean)

inetd-BF537_Debug_clean:
	-$(RM) "Debug\CharGen_Server_ThreadType.doj"
	-$(RM) "Debug\CharGen_Worker_ThreadType.doj"
	-$(RM) "Debug\Discard_Server_ThreadType.doj"
	-$(RM) "Debug\Discard_Worker_ThreadType.doj"
	-$(RM) "Debug\Echo_Server_ThreadType.doj"
	-$(RM) "Debug\Echo_Worker_ThreadType.doj"
	-$(RM) ".\Debug\ExceptionHandler-BF537.doj"
	-$(RM) "Debug\IPv4_util.doj"
	-$(RM) "Debug\lwip_sysboot_threadtype.doj"
	-$(RM) "Debug\VDK.doj"
	-$(RM) ".\Debug\inetd-BF537.dxe"
	-$(RM) ".\Debug\*.ipa"
	-$(RM) ".\Debug\*.opa"
	-$(RM) ".\Debug\*.ti"
	-$(RM) ".\Debug\*.pgi"
	-$(RM) ".\*.rbld"

endif


