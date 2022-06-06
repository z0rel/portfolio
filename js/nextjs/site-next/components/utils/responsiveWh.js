import site from "../constants/siteConstants";
import { routing } from "../routes/routing";

function invalidKeyException(message) {
    this.message = message;
    this.name = "invalidKeyException";
}

export const getImageFnameLazy = (uri, img) =>
    `${uri}${img.file_real}${img.ext}`;

export const getDataLazy = (uri, img, imgParams) => {
    let fname = getImageFnameLazy(uri, img);
    let keys = imgParams[fname];
    if (keys === undefined) {
        throw new invalidKeyException(`getDataLazy: ${getImageFnameLazy(uri, img)}`);
    }
    return {
        // datasrcz: `${site.domains.domain_canonical}${fname}`,
        ...keys,
    };
};

export const getImageFnameResp = (uri, img) => `${uri}${img.file}${img.ext}`;

export const getRespImgWh = (uri, img, imgParams) => {
    let fname = getImageFnameResp(uri, img);
    let keys = imgParams[fname];
    if (keys === undefined) {
        throw new invalidKeyException(`respImgWh: ${fname}`);
    }
    return {
        src: `${site.domains.domain_canonical}${fname}`,
        ...keys,
    };
};

export const imgWh = (fname, imgParams) => {
    let keys = imgParams[fname];
    if (keys === undefined) {
        throw new invalidKeyException(`respImgWh: ${fname}`);
    }
    return {
        src: `${site.domains.domain_canonical}${fname}`,
        ...keys,
    };
};

export const imgWz = (fname, imgParams) => {
    let keys = imgParams[fname];
    if (keys === undefined) {
        throw new invalidKeyException(`respImgWh: ${fname}`);
    }
    return {
        src: `${site.domains.domain_canonical}${fname}`,
        ...keys,
    };
};

export const imgIconWh = (iconName, imgParams) => {
    let fname = routing.path_sprites_png(iconName);
    let keys = imgParams[fname];
    if (keys === undefined) {
        throw invalidKeyException(`respImgWh: ${iconName} ${fname}`);
    }
    return {
        src: `${site.domains.domain_canonical}${fname}`,
        ...keys,
    };
};
