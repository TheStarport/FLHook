using System;
using System.Collections.Generic;
using System.Text;

namespace PlayerCntlSetup
{
    public interface AppServiceInterface
    {
        /// <summary>
        /// Ban the specified account.
        /// </summary>
        /// <param name="accDir">The account path</param>
        void BanAccount(string accDir, string accID, string banReason, DateTime banStart, DateTime banEnd);

        /// <summary>
        /// Unban the specified account.
        /// </summary>
        /// <param name="accDir">The account path</param>
        void UnbanAccount(string accDir);

        /// <summary>
        /// Save the specified char file. Kick/ban the player first if necessary.
        /// </summary>
        /// <param name="charFile"></param>
        void SaveCharFile(FLDataFile charFile);

        /// <summary>
        /// Set the filter to show the specified account directory.
        /// </summary>
        /// <param name="accDir"></param>
        void FilterOnAccDir(string accDir);
    }
}
