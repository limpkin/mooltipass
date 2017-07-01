package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Evernote;

public class EvernoteSteps {
	Evernote evernote= new Evernote(WebDriverFactory.get());

		@When("I login Evernote with '(.*)'")
		public void login(String username){
			//evernote.goToLogin();
			evernote.enterEmail(username);
			evernote.submit();
			String password =System.getenv().get("EVERPASS");
			evernote.enterPassword(password);
			evernote.submit();
			
		}
		@When("I go to Evernote login page")
		public void pressLogin(){
			WebDriverFactory.get().get("https://www.evernote.com/Login.action");
			Assert.assertTrue("Expected to be at login page", evernote.checkAtLoginPage());
			
		}
		@Then("I should be logged in Evernote")
		public void checkLogin(){
			Assert.assertTrue("Expected User to be logged in",evernote.checkLogin());
		}
		
		@When("I logout Evernote")
		public void pressLogout(){
			evernote.goTodDashboard();
			evernote.logout();
		}
}
