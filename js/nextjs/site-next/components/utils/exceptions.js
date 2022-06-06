export class UndefinedImgParams extends Error {
    constructor(message) {
        super(message);
        this.name = "UndefinedImgParams"
    }
}