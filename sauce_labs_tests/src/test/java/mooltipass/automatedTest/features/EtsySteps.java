package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Etsy;

public class EtsySteps {
	Etsy etsy= new Etsy(WebDriverFactory.get());

		@When("I login Etsy with '(.*)'")
		public void login(String username){
			etsy.goToLogin();
			etsy.enterEmail(username);
			String password =System.getenv().get("ETSYPASS");
			etsy.enterPassword(password);
			etsy.submit();
			
		}
		@When("I go to Etsy login page")
		public void pressLogin(){
			etsy.goToLogin();
			Assert.assertTrue("Expected to be at login page", etsy.checkAtLoginPage());
			
		}
		@Then("I should be logged in Etsy")
		public void checkLogin(){
			Assert.assertTrue("Expected User to be logged in",etsy.checkLogin());
		}
		
		@When("I logout Etsy")
		public void pressLogout(){
			etsy.goTodDashboard();
			etsy.logout();
		}
}
