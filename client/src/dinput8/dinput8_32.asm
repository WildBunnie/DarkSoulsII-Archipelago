; solution taken from https://stackoverflow.com/questions/12659972/distinguish-between-x64-and-x86-mode-in-masm
IFDEF RAX
    END_IF_NOT_X86 equ end
ELSE
    END_IF_NOT_X86 equ <>
ENDIF

END_IF_NOT_X86

.model flat, C
.code
extern mProcs:DWORD
DirectInput8Create_wrapper proc
	jmp mProcs[0*4]
DirectInput8Create_wrapper endp
DllCanUnloadNow_wrapper proc
	jmp mProcs[1*4]
DllCanUnloadNow_wrapper endp
DllGetClassObject_wrapper proc
	jmp mProcs[2*4]
DllGetClassObject_wrapper endp
DllRegisterServer_wrapper proc
	jmp mProcs[3*4]
DllRegisterServer_wrapper endp
DllUnregisterServer_wrapper proc
	jmp mProcs[4*4]
DllUnregisterServer_wrapper endp
GetdfDIJoystick_wrapper proc
	jmp mProcs[5*4]
GetdfDIJoystick_wrapper endp
end