; Check if compiling for x86 architecture
IFDEF RAX
    END_IF_NOT_X86 equ end
ELSE
    END_IF_NOT_X86 equ <>
ENDIF

END_IF_NOT_X86

.386
.model flat, c
.code
; thanks to pseudostripy for showing me how this is done
; and for creating a proof of concept version, making this possible

; this function allows us to get the itemlot id of an item that is being picked up
; since the itemGive function only receives the contents of the itemLot

; this is basically just recreating what happens in the function at 'DarkSoulsII.exe+25D230'
; which is the function responsible for calling the function that sets the map item as picked up
; we do this because this function is called AFTER the item is already added to the inventory
; so we "replicate" what it does so we can obtain the itemlot id before the item is added to inventory
get_pickup_id proc param_1:ptr, base_address:ptr
    
    push ebx
    push esi
    push edi

    mov edi, param_1
    mov ebx, base_address

    lea eax, [ebx + 25D3D0h] 
    call eax
    
    sub esp, 8
    lea esi, [esp]
    push esi
    push edi
    mov ecx, eax
    lea eax, [ebx + 25B870h]
    call eax
    
    test al,al
    jz done

    lea ecx, [esp]
    lea eax, [ebx + 2062B0h]
    call eax
    mov esi, [esp]
    add esp, 8
    
    test eax,eax
    jz done

    and esi, 0Fh
    dec esi
    jnz enemy_drop

    push eax
    lea eax, [ebx + 1C4BE0h]
    call eax
    add esp, 4
    mov esi, eax
    
    test esi, esi
    jz done

    jmp map_item

map_item:

    mov ecx, esi
    lea eax, [ebx + 25D1B0h]
    call eax
    
    test eax,eax
    jz chest_item

    mov eax, [eax + 28h]
    jmp done

chest_item:

    mov ecx, esi
    lea eax, [ebx + 25D1F0h]
    call eax

    test eax, eax
    jz done

    mov eax, [eax + 40h]
    jmp done

enemy_drop:

    push eax
    lea eax, [ebx + 25D170h]
    call eax
    add esp, 4

    test eax, eax
    jz done

    mov eax, [eax + 44h]
    mov eax, [eax + 80h]
    jmp done

done:
    pop edi
    pop esi
    pop ebx
    ret

get_pickup_id endp
end