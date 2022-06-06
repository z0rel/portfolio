import imageSize from 'image-size'

let getImgWh = (filename) => {
    let sizes = imageSize(filename)
    return ({
        width: sizes.width,
        height: sizes.height
    })
}

export default getImgWh;