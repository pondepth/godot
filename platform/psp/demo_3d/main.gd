extends Node3D

@onready var cube: RigidBody3D = $Cube
@onready var cube_mesh: MeshInstance3D = $Cube/Mesh


func _ready() -> void:
	var image := Image.create(64, 64, false, Image.FORMAT_RGBA8)
	for y in range(64):
		for x in range(64):
			var checker := (int(x / 8) + int(y / 8)) % 2
			image.set_pixel(x, y, Color("e9b44c") if checker == 0 else Color("344966"))

	var material := StandardMaterial3D.new()
	material.albedo_texture = ImageTexture.create_from_image(image)
	material.albedo_color = Color.WHITE
	cube_mesh.material_override = material
	$Camera3D.look_at(Vector3(0, 0.8, 0))


func _physics_process(_delta: float) -> void:
	var analog_x := Input.get_joy_axis(0, JOY_AXIS_LEFT_X)
	var analog_y := Input.get_joy_axis(0, JOY_AXIS_LEFT_Y)
	if abs(analog_x) < 0.15:
		analog_x = Input.get_axis("ui_left", "ui_right")
	if abs(analog_y) < 0.15:
		analog_y = Input.get_axis("ui_up", "ui_down")

	cube.apply_torque(Vector3(analog_y * 4.0, 1.2 + analog_x * 4.0, -analog_x * 2.0))
	if cube.position.y < -4.0:
		cube.position = Vector3(0, 2.4, 0)
		cube.linear_velocity = Vector3.ZERO
		cube.angular_velocity = Vector3.ZERO
