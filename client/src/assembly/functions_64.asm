; Check if compiling for x64 architecture
IFDEF RAX
    END_IF_NOT_X64 equ <>
ELSE
    END_IF_NOT_X64 equ end
ENDIF

END_IF_NOT_X64

.code

; thanks to pseudostripy for showing me how this is done
; and for creating a proof of concept version, making this possible

; this function allows us to get the itemlot id of an item that is being picked up
; since the itemGive function only receives the contents of the itemLot

; this is basically just recreating what happens in the function at 'DarkSoulsII.exe+1E5DD0'
; which is the function responsible for calling the function that sets the map item as picked up
; we do this because this function is called AFTER the item is already added to the inventory
; so we "replicate" what it does so we can obtain the itemlot id before the item is added to inventory
get_pickup_id proc 
    
    push rcx
    push rdx
    push rbx
    push rbp
    push r12
    push r13
    mov rbp, rsp
    sub rsp, 200h

    mov r12, rcx ; param_1 
    mov rbx, rdx ; base_address

    lea rax, [rbx + 1E6550h] 
    call rax

    lea r8, [rbp - 100h]
    mov rdx, r12
    mov rcx, rax
    lea rax, [rbx + 1E3E60h] 
    call rax

    test al,al
    jz done

    lea rcx, [rbp - 100h]
    lea rax, [rbx + 17B830h]
    call rax

    test rax, rax
    jz done

    mov edx, [rbp - 100h]
    and edx, 0Fh
    dec edx
    jnz enemy_drop

    mov rcx, rax
    lea rax, [rbx + 133AD0h]
    call rax
    mov r13, rax

    test r13, r13
    jz done

    jmp map_item

map_item:
    mov rcx, r13                   
    lea rax, [rbx + 1E5C10h]
    call rax

    test rax,rax
    jz chest_item

    mov eax, [rax+48h]
    jmp done

chest_item:
    mov rcx,r13
    lea rax, [rbx + 1E5C80h]
    call rax

    test rax,rax
    jz done

    mov eax, [rax+70h]
    jmp done

enemy_drop:
    mov rcx,rax
    lea rax, [rbx + 1E5BB0h]
    call rax

    test rax,rax
    jz done

    mov rax, [rax+68h]
    mov eax, [rax+90h]
    jmp done

done:
    mov rsp, rbp
    pop r13
    pop r12
    pop rbp
    pop rbx
    pop rdx
    pop rcx

    ret

get_pickup_id endp
end