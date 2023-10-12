subroutine display(status, n)
    implicit none 
    integer, dimension(35, 35), intent(inout) :: status
    integer, intent(inout) :: n
    character(70) :: string
    integer :: i, j

    do i = 1, n 
        string = ' '
        do j = 1, n
            if (status(i, j) == 1) then 
                string = trim(string)//' @ '
            else 
                string = trim(string)//' . '
            end if
        end do
        print *, string
    end do
end subroutine display