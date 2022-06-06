export const mk = (str, styles) => {
    let sarr = str.split(' ')
    let arr = sarr.map((className) => styles[className]);
    return `${arr.join(' ')}`;
}

export const mkOrEmpty = (className, styles) => {
    let value = styles[className];
    if (value === undefined) {
        if (className === undefined) {
            return ''
        }
        return className;
    }
    return value;
}
