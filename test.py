from gpiozero.pins import Factory

factory = Factory();
print(f'{factory.board_info:full}')