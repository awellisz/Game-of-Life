subroutine display(state, n)
    implicit none 

    integer, dimension(35, 35), intent(inout) :: state
    integer, intent(inout) :: n
    character(70) :: string
    integer :: i, j

    do i = 1, n 
        string = ' '
        do j = 1, n
            if (state(i, j) == 1) then 
                ! Trim whitespace and concatenate with @
                string = trim(string)//' @ ' 
                !string = trim(string)//' '//achar(27)//'[31m@ '//achar(27)//'[0m.'//' '
            else 
                string = trim(string)//' . '
            end if
        end do
        print *, string
    end do

    call sleep(1) ! one second! find a better way

end subroutine display

program life
    implicit none

    integer, dimension(35, 35) :: state, next 
    integer :: n, ran, i, j, nseed
    real :: rn1

    n = 35

    print *, "Enter 1 for manual setup, 0 for random."
    read(*,*)ran 
    !print *, 'A great color is '//achar(27)//'[95m pink '//achar(27)//'[0m.'

    ! Set up the board 
    do i = 1, n 
        do j = 1, n 
            state(i,j) = 0
            next(i, j) = 0
        end do
    end do

    ! Initialize live cells 
    if (ran == 1) then 
        do while(.true.)
            print *,"ENTER COORDINATES OF LIVE CELLS; WRITE 0,0 WHEN DONE"
            read(*,*)i,j
            if((i == 0) .and. (j == 0)) exit
            state(i, j) = 1
            call display(state, n)
        end do
    else 
        print *,"ENTER A FIVE DIGIT INTEGER TO CHANGE SEED"
        read(*,*)nseed
        call srand(nseed)

        do i = 1, n 
            do j = 1, n 
                rn1 = rand()
                if (rn1 < 0.75) then
                    state(i,j) = 0
                else 
                    state(i,j) = 1
                end if
            end do
        end do 
    end if
    
    call display(state, n)
    print *,"ADJUST SCREEN TO BE 36 LINES LONG"

    ! RULES FOR CELLS !

    ! Loop over all cells
    do i = 1, n 
        do j = 1, n 
            ! Find currently living cells
            ! if (state(i,j) == 1) then
        end do
    end do


end program life 

subroutine count_neighbors(state, i, j, num_neighbors) 
    integer, dimension(35, 35), intent(in) :: state
    integer, intent(in) :: i, j 
    integer, intent(inout) :: num_neighbors

    ! The state array is either 1 or 0 for alive/dead; simply add the cells
    num_neighbors = state(i-1,j-1) + state(i,j-1) + state(i+1,j-1) &
                    + state(i-1,j) + state(i+1,j) &
                    + state(i-1,j+1) + state(i,j+1) + state(i+1,j+1)
    
end subroutine count_neighbors