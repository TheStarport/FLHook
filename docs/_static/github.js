import { Octokit } from "https://esm.sh/@octokit/core";

const getContributors = async () => {
    const octo = new Octokit();
    const result = await octo.request('GET /repos/{owner}/{repo}/contributors', {
        owner: 'TheStarport',
        repo: 'FLHook'
    });

    const contributors = [];
    for (let i = 0; i < result.data.length; i++) {
        contributors.push({
            login: result.data[i].login,
            avatar_url: result.data[i].avatar_url,
            html_url: result.data[i].html_url,
            contributions: result.data[i].contributions,
        });
    }

    return contributors;
}

const getLatestRelease = async () => {
    const octo = new Octokit();
    const result = await octo.request('GET /repos/{owner}/{repo}/releases', {
        owner: 'TheStarport',
        repo: 'FLHook'
    });

    return result.data[0];
}

document.addEventListener("DOMContentLoaded", async function() {
    if (window.location.href.includes("contributors")) {

        const topEl = document.getElementById("github-contributors");
        if (!topEl) {
            return;
        }

        const contributors = await getContributors();
        contributors.sort((a, b) => {
            return a.contributions < b.contributions;
        });

        const container = document.createElement("div");
        container.classList.add("sd-row", "sd-row-cols-2", "sd-row-cols-xs-2", 
            "sd-row-cols-sm-2", "sd-row-cols-md-3", "sd-row-cols-lg-3", "docutils");

        const elements = contributors.map(contributor => {
            return `
                <div class="sd-col sd-d-flex-row docutils">
                    <div class="sd-card sd-sphinx-override sd-w-100 sd-shadow-sm docutils">
                        <div class="sd-card-body docutils sd-d-flex-column sd-align-minor-center">
                            <img src="${contributor.avatar_url}" href="${contributor.html_url}" class="sd-avatar-md">
                            <div class="text-sm sd-text-center">
                                <p class="sd-font-weight-bold">
                                    <a href=${contributor.html_url}>
                                        ${contributor.login}
                                    </a>
                                </p>
                                <p>${contributor.contributions} commits</p>
                            </div>
                        </div>
                    </div>
                </div>
            `;
        });

        container.innerHTML = elements.join('\n');
        topEl.appendChild(container);
    }
    else if (window.location.href.includes("downloads")) {
        const topEl = document.getElementById("latest-release");
        if (!topEl) {
            return;
        }

        const latestRelease = await getLatestRelease();
        topEl.innerHTML = `
            <h2>Latest Release</h2>
            <p>
                ${latestRelease.body}<br/><br/>
                Click <a href="${latestRelease.assets[0].browser_download_url}" target="_blank">here</a> to download.
            </p>
            <p>
                This was created on ${new Date(latestRelease.created_at).toUTCString()}.<br/>
            </p>
        `;
    }
});