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

; this function allows us to get the itemLotId of an item that is being picked up
; since the itemGive function only receives the contents of the itemLot
getItemLotId proc 
    
    push rcx
    push rdx
    push rbx
    push rbp
    push r12
    mov rbp, rsp
    sub rsp, 200h

    mov r12, r8 ; baseAddress

    mov rax, qword ptr [rdx]

    shl rax, 10h
    sar rax, 30h

    mov rcx, [rcx+8h]
    imul rax, rax, 8h
    add rax,rcx
    mov rcx, [rax]

    lea rax, [r12 + 1e04e0h]
    call rax
    
    mov rax, [rax + 120h]
    mov [rbp-200h], rax

    ; return if it's an item we dropped
    test rax,rax
    jz done

    lea rcx, [rbp-200h]
    lea rax, [r12 + 17b830h]
    call rax

    mov rbx, rax
    jmp is_map_item

is_map_item:
    mov rcx, rbx                   
    lea rax, [r12 + 1e5c10h]
    call rax

    test rax,rax
    jz is_chest

    mov eax, [rax+48h]
    jmp done

is_chest:
    mov rcx,rbx
    lea rax, [r12 + 1e5c80h]
    call rax

    test rax,rax
    jz is_enemy_drop

    mov eax, [rax+70h]
    jmp done

is_enemy_drop:
    mov rcx,rbx
    lea rax, [r12 + 1e5bb0h]
    call rax

    test rax,rax
    jz error

    mov rax, [rax+68h]
    mov eax, [rax+90h]
    jmp done

error:
    mov eax, -1
    jmp done

done:
    mov rsp, rbp
    pop r12
    pop rbp
    pop rbx
    pop rdx
    pop rcx

    ret

getItemLotId endp
end