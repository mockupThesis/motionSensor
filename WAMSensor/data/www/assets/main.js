var scene, camera, renderer, socket

var WIDTH = window.innerWidth - 100
var HEIGHT = window.innerHeight - 100

Math.radians = function(degrees) {
  return degrees * Math.PI / 180;
}

function init()
{
    scene = new THREE.Scene()
    initCube()
    initCamera()
    initRenderer()
    document.body.appendChild(renderer.domElement)
    socket = new WebSocket('ws://wamsensor.local/ws')
    socket.onopen = function(e) {
		console.log('SOCKET OPENED')
		try {
			console.log('SOCKET OPEN SUCCESS')
		} catch(e) {
			console.log(e)
		}
    }
    socket.onclose = function() {
        console.log('SOCKET CLOSE')
    }
    socket.onmessage = function(e) {
		console.log('ONMESSAGE')
        if (e && e.data)
        {
            console.log(e.data)
            let coords = e.data.split("\t")
            console.log(coords)
            cube.rotation.x = Math.radians(parseFloat(coords[0]))
            cube.rotation.y = Math.radians(parseFloat(coords[1]))
            cube.rotation.z = Math.radians(parseFloat(coords[2]))
        }

    }
    setInterval(function(e) {
        try {
			console.log('SEND G')
            socket.send('G')
        }  catch(e) {
			console.log(e)
		}
    }, 100)

}

function initCube()
{
	console.log('INIT CUBE')
    let material = new THREE.MeshBasicMaterial({
        color: 0xffffff,
        vertexColors: THREE.FaceColors
    })

    let geometry = new THREE.BoxGeometry(8, .5, 3)

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
