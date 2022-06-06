import getImgWh from "../utils/getImgWh";
import * as path from 'path';

const getImagesParams = (filenames) => {
    let map = {}
    for (let fname of filenames) {
        map[fname] = getImgWh(path.join('public', fname))
    }
    return map;
}

export default getImagesParams