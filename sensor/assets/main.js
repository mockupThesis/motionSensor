var scene, camera, renderer, socket

var WIDTH = window.innerWidth - 100
var HEIGHT = window.innerHeight - 100

var X
var Y
var Z

Math.radians = function(degrees) {
  return degrees * Math.PI / 180;
}

/*quaternionToEuler = function(w, x, y, z) {
    ysqr = y * y
	
	t0 = 2.0 * (w * x + y * z)
	t1 = 1.0 - 2.0 * (x * x + ysqr)
    X = Math.radians(Math.atan(t0, t1))
	
    t2 = 2.0 * (w * y - z * x)
    if(t2 > 1.0) {
        t2 = 1.0
    }
    if(t2 < -1.0) {
        t2 = -1.0
    }
    Y = Math.radians(Math.asin(t2))
	
	t3 = 2.0 * (w * z + x * y)
	t4 = 1.0 - 2.0 * (ysqr + z * z)
    Z = Math.radians(Math.atan(t3, t4))
}*/

function init()
{
    scene = new THREE.Scene()
    initCube()
    initCamera()
    initRenderer()
    document.body.appendChild(renderer.domElement)
    socket = new WebSocket('ws://wamsensor.local/ws')
    socket.onopen = function() {
		console.log('SOCKET OPENED')
    }
    socket.onclose = function() {
        console.log('SOCKET CLOSE')
    }
    socket.onmessage = function(e) {
        if (e && e.data)
        {
            console.log('DATA', e)
            let coords = e.data.split("\t")
            //console.log('COORDS', coords[0])
            
            cube.rotation.x = Math.radians(parseFloat(coords[1]))
            cube.rotation.y = Math.radians(parseFloat(coords[0]))
            cube.rotation.z = Math.radians(parseFloat(coords[2]))
            /*quaternionToEuler(coords[0], coords[1], coords[2], coords[3])
            console.log('X', X)
            console.log('Y', Y)
            console.log('Z', Z)
            cube.rotation.x = X
            cube.rotation.y = Y
            cube.rotation.z = Z*/
        }

    }
    setInterval(function(e) {
        try {
            socket.send('G')
        }  catch(e) {
			console.log(e)
		}
    }, 100)

}

function initCube()
{
    let material = new THREE.MeshBasicMaterial({
        color: 0xffffff,
        vertexColors: THREE.FaceColors
    })

    //let geometry = new THREE.BoxGeometry(8, .5, 3)
    let geometry = new THREE.BoxGeometry(5, 5, 5)

    red = new THREE.Color(1, 0, 0)
    green = new THREE.Color(0, 1, 0)
    blue = new THREE.Color(0, 0, 1)
    const colors = [red, green, blue]

    for (let i = 0; i < 3; i++) {
        geometry.faces[4 * i].color = colors[i]
        geometry.faces[4 * i + 1].color = colors[i]
        geometry.faces[4 * i + 2].color = colors[i]
        geometry.faces[4 * i + 3].color = colors[i]
    }

    cube = new THREE.Mesh(geometry, material)
    scene.add(cube)
}

function initCamera()
{
    camera = new THREE.PerspectiveCamera(70, WIDTH / HEIGHT, 1, 50)
    camera.position.set(0, 5, 10)
    camera.lookAt(scene.position)
}

function initRenderer()
{
    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(WIDTH, HEIGHT)
}

function render()
{
    requestAnimationFrame(render)

    // cube.rotation.x -= 0.01 * 2

    renderer.render(scene, camera)
}

init()
render()
