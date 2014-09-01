/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/* \file    AutoLoginAft.java
 * \brief   Automatic Functional Test using Selenium and BETATESTERS_AUTOACCEPT_SETUP hex
 *  Created: 10/08/2014 13:29:42
 *  Author: Erik G. H. Meade
 */
package mooltipass;

import static org.junit.Assert.*;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.LinkedList;
import java.util.List;

import org.junit.BeforeClass;
import org.junit.Test;
import org.openqa.selenium.By;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.ui.ExpectedConditions;
import org.openqa.selenium.support.ui.WebDriverWait;

/**
 * Tests with BETATESTERS_AUTOACCEPT_SETUP hex us a file where each line is: url,login_link_text,logout_link_text
 * Set -Dmooltipass.auto.login.file=full_path_to_file
 * 
 *  OR
 *  
 * Specify a line (replacing spaces with underscores) on the command line using -Dmooltipass.auto.login.line=URL,Login_Text,Logout_Text
 *
 * @See AutoLoginTestData.txt
 *
 * @author eghm
 */
public class AutoLoginAft extends AftBase {
	public static final Character DELIMITER = ',';
	public static final String MOOLTIPASS_AUTO_LOGIN_FILE = "mooltipass.auto.login.file";
	public static final String MOOLTIPASS_AUTO_LOGIN_LINE = "mooltipass.auto.login.line";
	public static List<String> tests = new LinkedList<String>();

	@BeforeClass
	public static void beforeClass() throws Exception {
		String loginLine = System.getProperty(MOOLTIPASS_AUTO_LOGIN_LINE);
		if (System.getProperty(MOOLTIPASS_AUTO_LOGIN_FILE) == null
				&& loginLine == null)
		{
			System.err.println("-D" + MOOLTIPASS_AUTO_LOGIN_FILE
					+ "= must be set to login file");
			System.err.println("\tOR");
			System.err.println("-D" + MOOLTIPASS_AUTO_LOGIN_LINE
					+ "=\"URL, [Login], [Logout]\"");
			System.exit(1);
		}
		else if (loginLine != null)
		{
			tests.add(loginLine.replaceAll("_", " "));
		}
		else
		{
			BufferedReader br = new BufferedReader(new FileReader(
					System.getProperty(MOOLTIPASS_AUTO_LOGIN_FILE)));
			try
			{
				String line = "";
				while ((line = br.readLine()) != null)
				{
					if (!line.startsWith("#")) // comment 
					{
						tests.add(line + System.lineSeparator());
					}
				}
			}
			finally
			{
				if (br != null) 
				{
					br.close();					
				}
			}
		}
		AftBase.beforeClass();
	}

	@Test
	public void testAutoLogin() throws Exception {
		boolean passed = true;
		for (String test : tests) {
			System.out.print("testing: " + test.split(",")[0]);
			try
			{
				testAutoLogin(test);
				System.out.print(" PASS");
			}
			catch (Throwable t)
			{
				passed = false;
				System.out.print(" FAIL " + t.getMessage());
//				t.printStackTrace();
			}
			screenshot();				
			System.out.println();
		}
		assertTrue(passed);
	}

	void testAutoLogin(String data) throws Exception
	{
		testAutoLogin(data.split(DELIMITER.toString()));
	}
	
	void testAutoLogin(String[] data) throws Exception
	{
		switch (data.length)
		{
			case 1:
				testAutoLogin(data[0].trim(), null, null);
				break;
			case 2:
				testAutoLogin(data[0].trim(), data[1].trim(), null);
				break;
			case 3: 
				testAutoLogin(data[0].trim(), data[1].trim(), data[2].trim());
				break;
				
			default:
				StringBuilder builder = new StringBuilder();
				for (int i = 0; i < data.length; i++)
				{
					builder.append(data[i]).append(DELIMITER);
				}
				System.err.println("Input not formatted as expected: " + builder.toString().substring(0, builder.length() -1)); // remove trailing delimiter
				break;
		}
	}

	/**
	 * TODO: check for buttons if links aren't present
	 * 
	 * @param loginUrl
	 * @param loginLoadedLinkText - link text used to determine when page has loaded
	 * @param logoutLinkText - link text to click to log out at end of test
	 * @throws Exception
	 */
	void testAutoLogin(String loginUrl, String loginLoadedLinkText, String logoutLinkText) throws Exception
	{
		driver.get(loginUrl);
		WebDriverWait wait = new WebDriverWait(driver, getTimeout());
		// wait for Login, but we are not gonna click it testing AUTOACCEPT works.
		// TODO fix assumption that all sites have a direct Login page, but not all do.  
		if (loginLoadedLinkText != null &&  !loginLoadedLinkText.equals(""))
		{
			wait.until(ExpectedConditions.elementToBeClickable(By.linkText(loginLoadedLinkText)));
		}
		else
		{
			wait.until(ExpectedConditions.presenceOfElementLocated(By.xpath("/html")));
		}

		screenshot(); // of given loginUrl
		
		// various in page login pop-ups are detected by MP and can be submitted without clicking the link
		// that makes them visible.
		// TODO handle the other cases
		if (loginLoadedLinkText != null &&  !loginLoadedLinkText.equals(""))
		{ 
			wait.until(ExpectedConditions.elementToBeClickable(By.linkText(loginLoadedLinkText)));
		}

		Thread.sleep(1000);

		if (logoutLinkText != null &&  !logoutLinkText.equals(""))
		{
			WebElement logout = wait.until(ExpectedConditions.elementToBeClickable(By.linkText(logoutLinkText)));
//			logout.click(); // clicking log out is nearly useless, MP will log back in immediately with AUTOACCEPT
		}
		else
		{
			wait.until(ExpectedConditions.presenceOfElementLocated(By.xpath("/html")));
		}
		screenshot();
	}
}
