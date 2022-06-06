import Link from "next/link";

const InternalLinkWorkNobr = ({uri, name}) => {
    return <Link href={uri} passHref>
        <a>
            <span class='abbr' data-toggle="tooltip" data-placement="top" title="Перейти к странице выполненного проекта">
                <span class="no-break-nobr" dangerouslySetInnerHTML={{__html: name}}></span>
            </span>
        </a>
    </Link>
}

export default InternalLinkWorkNobr;