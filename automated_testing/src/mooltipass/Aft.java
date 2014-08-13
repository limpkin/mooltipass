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
/* \file    Aft.java
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

/**
 * Tests with BETATESTERS_AUTOACCEPT_SETUP hex us a file where each line is: url,login_link_text,logout_link_text
 * Set -Dmooltipass.auto.login.file=full_path_to_file
 *
 * @author eghm
 */
public class Aft extends AftBase {
	public static final String MOOLTIPASS_AUTO_LOGIN_FILE = "mooltipass.auto.login.file";
	public static List<String> tests = new LinkedList<String>();

	@BeforeClass
	public static void beforeClass() throws Exception {
		if (System.getProperty(MOOLTIPASS_AUTO_LOGIN_FILE) == null) {
			System.err.println("-D" + MOOLTIPASS_AUTO_LOGIN_FILE
					+ "= must be set to login file");
			System.exit(1);
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
					tests.add(line + System.lineSeparator());
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
			catch (Throwable t) {
				System.out.print(" FAIL");
				passed = false;
			}
			System.out.println();
		}
		assertTrue(passed);
	}
}
